import * as ts from 'typescript';
import * as path from 'path';
import { TsError } from './error';
import { BindgenOptions, namespacedName } from './utils';
import { Templatizer } from './template';

interface ScriptClassProperty {
    name: string;
    readonly: boolean;
    type: string;
    get?: boolean;
    set?: boolean;
}

interface ScriptClass {
    namespace: string[];
    file: string;
    name: string;
    isInterface: boolean;
    ctor?: ScriptConstructor;
    partial: boolean;
    properties: ScriptClassProperty[];
    methods: ScriptMethod[];
}

interface ScriptEnum {
    namespace: string[];
    file: string;
    name: string;
    members: string[];
}

interface ScriptFunctionArg {
    name: string;
    type: string;
    partial: boolean;
}

interface ScriptConstructor {
    params: ScriptFunctionArg[];
}

interface ScriptMethod {
    name: string;
    params: ScriptFunctionArg[];
    optionalArgs?: number;
    returnType: string;
}

type ScriptFunction = ScriptMethod & {
    namespace: string[];
    file: string;
    jsfunc?: boolean;
};

export interface ScriptTypes {
    classes: ScriptClass[];
    enums: ScriptEnum[];
    functions: ScriptFunction[];
    hasDeclarations: boolean;
}

const validationTypes = {
    string: 'std::string',
    number: 'double',
    boolean: 'bool',
    void: 'void',
    any: 'JSValue',
    ArrayBuffer: 'JSValue',
};

export class Bindgen {
    program: ts.Program;
    checker: ts.TypeChecker;
    scriptTypes: Record<string, ScriptTypes> = {};
    currentNamespace: string[] = [];
    errors: TsError[] = [];

    constructor(public options: BindgenOptions) {
        this.program = ts.createProgram(options.filePaths, {
            target: ts.ScriptTarget.ES2020,
            lib: ['lib.es2020.d.ts'],
        });
        // this is needed for source positions to work
        this.checker = this.program.getTypeChecker();
    }

    getNamespace() {
        return this.currentNamespace.slice();
    }

    namespacedName(o: { file: string; name: string }) {
        return namespacedName(this.options, o);
    }

    parseType(node: ts.TypeNode, access?: string) {
        if (ts.isArrayTypeNode(node)) {
            const elementType = this.parseType(node.elementType, access ? `${access}::value_type` : undefined);
            return { access, validationType: `std::vector<${elementType.validationType}>` };
        } else if (ts.isFunctionTypeNode(node)) {
            return { access, validationType: 'JSValue' };
        } else if (ts.isTypeReferenceNode(node) && node.typeName.getText() === 'Array') {
            if (node.typeArguments?.length !== 1) {
                throw new TsError('invalid Array type: should have one type argument', node);
            }
            const elementType = this.parseType(node.typeArguments[0], access ? `${access}::value_type` : undefined);
            return { access, validationType: `std::vector<${elementType.validationType}>` };
        } else if (ts.isLiteralTypeNode(node)) {
            throw new TsError('literal types are not currently supported', node);
        } else if (ts.isOptionalTypeNode(node)) {
            const elementType = this.parseType(node.type, access ? `${access}::value_type` : undefined);
            return { access, validationType: `std::optional<${elementType.validationType}>` };
        } else if (ts.isUnionTypeNode(node)) {
            if (node.types.length === 2) {
                // optional
                for (let i = 0; i < 2; ++i) {
                    if (node.types[i].getText() === 'undefined') {
                        const inner = this.parseType(node.types[1 - i], access ? `${access}::value_type` : undefined);
                        return { access, validationType: `std::optional<${inner.validationType}>` };
                    }
                }
            }
        } else if (ts.isTypeReferenceNode(node)) {
            const t = node.typeName.getText();
            if (validationTypes[t]) {
                return { access, validationType: validationTypes[t] };
            }
            const type = this.checker.getTypeFromTypeNode(node);
            const intrinsicName = (type as any).intrinsicName;
            if (intrinsicName === 'error') {
                throw new TsError(`type resolution failed`, node);
            }
            if (type.symbol) {
                return { access, validationType: type.symbol.getName() };
            }
        }
        const s = node.getText();
        if (validationTypes[s]) {
            return { access, validationType: validationTypes[s] };
        }
        throw new TsError(`unsupported type (node kind: ${node.kind})`, node);
    }

    parseProperty(node: ts.PropertyDeclaration | ts.PropertySignature, access?: string) {
        let isStatic = false;
        let isReadOnly = false;
        if (node.modifiers) {
            for (const mod of node.modifiers) {
                switch (mod.kind) {
                    case ts.SyntaxKind.StaticKeyword: {
                        isStatic = true;
                        break;
                    }
                    case ts.SyntaxKind.ReadonlyKeyword: {
                        isReadOnly = true;
                        break;
                    }
                }
            }
        }
        return {
            name: node.name.getText(),
            type: this.parseType(node.type, access),
            readonly: isReadOnly,
            static: isStatic,
        };
    }

    parseFunctionArg(node: ts.ParameterDeclaration, access?: string) {
        let type = node.type;
        let partial = false;
        if (ts.isTypeReferenceNode(type) && type.typeName.getText() === 'Partial') {
            if (type.typeArguments.length === 1) {
                partial = true;
                type = type.typeArguments[0];
            } else {
                throw new TsError('invalid Partial type; should have one type argument', node);
            }
        }
        return {
            name: node.name.getText(),
            type: this.parseType(type, access),
            partial,
        };
    }

    parseMethod(node: ts.FunctionDeclaration | ts.MethodDeclaration | ts.MethodSignature, access?: string) {
        let isStatic = false;
        if (node.modifiers) {
            for (const mod of node.modifiers) {
                switch (mod.kind) {
                    case ts.SyntaxKind.StaticKeyword: {
                        isStatic = true;
                        break;
                    }
                }
            }
        }
        return {
            name: node.name.getText(),
            optionalArgs: node.parameters.reduce((a, b) => a + (b.questionToken ? 1 : 0), 0),
            params: node.parameters.map((arg, i) =>
                this.parseFunctionArg(
                    arg,
                    access ? `std::tuple_element<${i}, FunctionInfo<${access}>::ArgTypes>::type` : undefined,
                ),
            ),
            returnType: this.parseType(node.type, access ? `FunctionInfo<${access}>::ReturnType` : undefined),
            static: isStatic,
        };
    }

    parseFunction(node: ts.FunctionDeclaration | ts.MethodDeclaration, access?: string) {
        const f: ScriptFunction = {
            ...this.parseMethod(node, access),
            namespace: this.getNamespace(),
            file: node.getSourceFile().fileName,
        };
        if (this.hasTag(node, 'jsfunc')) {
            f.jsfunc = true;
        }
        return f;
    }

    parseConstructor(node: ts.ConstructorDeclaration, access?: string) {
        return {
            params: node.parameters.map((arg, i) =>
                this.parseFunctionArg(
                    arg,
                    access ? `std::tuple_element<${i}, FunctionInfo<${access}>::ArgTypes>::type` : undefined,
                ),
            ),
        };
    }

    hasTag(n: ts.Node, tagName: string) {
        for (const tag of ts.getJSDocTags(n)) {
            if (tag.tagName.text === tagName) {
                return true;
            }
        }
        return false;
    }

    execute() {
        for (const file of this.program.getSourceFiles()) {
            const filePath = path.resolve(file.fileName);
            if (!filePath.startsWith(this.options.srcDir)) {
                continue;
            }

            const types: ScriptTypes = (this.scriptTypes[path.relative(this.options.srcDir, filePath)] = {
                classes: [],
                enums: [],
                functions: [],
                hasDeclarations: false,
            });

            const visitor = (node: ts.Node) => {
                try {
                    if (this.hasTag(node, 'skip')) {
                        return;
                    }
                    if (ts.isModuleDeclaration(node)) {
                        // namespace
                        this.currentNamespace.push(node.name.text);
                        visit(node.body);
                        this.currentNamespace.pop();
                    } else if (ts.isClassDeclaration(node) || ts.isInterfaceDeclaration(node)) {
                        // classes and interfaces: generate a new script class
                        const cls = {
                            namespace: this.getNamespace(),
                            file: file.fileName,
                            name: node.name.text,
                            isInterface: ts.isInterfaceDeclaration(node),
                            properties: [],
                            staticProperties: [],
                            methods: [],
                            staticMethods: [],
                            ctor: undefined,
                            partial: false,
                            clone: false,
                            parent: undefined,
                        };
                        if (!cls.isInterface && node.heritageClauses?.length) {
                            for (const clause of node.heritageClauses) {
                                // handle `extends`
                                for (const t of clause.types) {
                                    const type = this.checker.getTypeFromTypeNode(t);
                                    if (type.symbol) {
                                        for (const decl of type.symbol.declarations) {
                                            if (ts.isClassDeclaration(decl)) {
                                                cls.parent = {
                                                    file: decl.getSourceFile().fileName,
                                                    name: decl.name.getText(),
                                                };
                                            }
                                        }
                                    }
                                }
                                if (!cls.parent) {
                                    throw new TsError("couldn't process heritage clause", clause);
                                }
                            }
                        }
                        ts.forEachChild(node, (node: ts.Node) => {
                            if (this.hasTag(node, 'skip')) {
                                return;
                            }
                            if (ts.isPropertyDeclaration(node) || ts.isPropertySignature(node)) {
                                // property
                                const prop = this.parseProperty(
                                    node,
                                    `decltype(${this.namespacedName(cls)}::${node.name.getText()})`,
                                );
                                (prop.static ? cls.staticProperties : cls.properties).push(prop);
                            } else if (ts.isGetAccessorDeclaration(node) || ts.isSetAccessorDeclaration(node)) {
                                // property getter/setter
                                const p = ts.isGetAccessorDeclaration(node) ? 'get' : 'set';
                                const name = node.name.getText();
                                const type = this.parseType(p === 'get' ? node.type : node.parameters[0].type);
                                let found: ScriptClassProperty | undefined;
                                for (const prop of cls.properties) {
                                    if (prop.name === name && typeof prop.type === 'object') {
                                        if (prop[p]) {
                                            throw new TsError(`property ${name} already has a ${p}ter`, node);
                                        }
                                        found = prop;
                                        break;
                                    }
                                }
                                if (found) {
                                    if (p === 'set') {
                                        found.readonly = false;
                                        found.set = true;
                                    }
                                } else {
                                    cls.properties.push({ name, type, [p]: true, readonly: p === 'get' });
                                }
                            } else if (ts.isMethodDeclaration(node) || ts.isMethodSignature(node)) {
                                // method
                                if (node.name.getText() === 'clone') {
                                    cls.clone = true;
                                } else {
                                    const method = this.parseMethod(
                                        node,
                                        `decltype(&${this.namespacedName(cls)}::${node.name.getText()})`,
                                    );
                                    (method.static ? cls.staticMethods : cls.methods).push(method);
                                }
                            } else if (ts.isConstructorDeclaration(node)) {
                                const isPartial =
                                    node.parameters.length === 1 &&
                                    ts.isTypeReferenceNode(node.parameters[0].type) &&
                                    node.parameters[0].type.typeName.getText() === 'Partial';
                                if (isPartial) {
                                    // partial constructor
                                    cls.partial = true;
                                } else {
                                    // constructor
                                    if (cls.ctor) {
                                        throw new TsError('multiple constructors are not supported', node);
                                    }
                                    cls.ctor = this.parseConstructor(
                                        node,
                                        `decltype(&${this.namespacedName(cls)}::create)`,
                                    );
                                }
                            }
                        });
                        if (cls.partial && !cls.ctor) {
                            // must have a default constructor
                            cls.ctor = { params: [] };
                        }
                        for (const prop of cls.properties) {
                            if (prop.get && !prop.set) {
                                prop.readonly = true;
                            }
                        }
                        types.classes.push(cls);
                    } else if (ts.isEnumDeclaration(node)) {
                        const e = {
                            namespace: this.getNamespace(),
                            file: file.fileName,
                            name: node.name.text,
                            members: node.members.map((member) => member.name.getText()),
                        };
                        types.enums.push(e);
                    } else if (ts.isFunctionDeclaration(node)) {
                        // functions
                        const f = this.parseFunction(
                            node,
                            this.hasTag(node, 'jsfunc')
                                ? undefined
                                : `decltype(&${this.namespacedName({
                                      file: file.fileName,
                                      name: node.name.getText(),
                                  })})`,
                        );
                        types.functions.push(f);
                    }
                } catch (e) {
                    if (e instanceof TsError) {
                        this.errors.push(e);
                    } else {
                        this.errors.push(new TsError(e.stack, node));
                    }
                }
            };
            const visit = (node: ts.Node) => ts.forEachChild(node, visitor);
            visit(file);

            types.hasDeclarations = !!(types.classes.length || types.enums.length || types.functions.length);
        }

        if (this.errors.length) {
            for (const err of this.errors) {
                console.error(err.toString());
            }
            console.error(`Binding generation failed: there were ${this.errors.length} errors`);
            process.exit(1);
        }

        const templatizer = new Templatizer(this.options);
        for (const file in this.scriptTypes) {
            templatizer.generateTemplates(file, this.scriptTypes[file]);
        }
    }
}
