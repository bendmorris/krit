import * as ts from 'typescript';
import * as path from 'path';
import { TsError } from './error';
import { BindgenOptions, namespacedName } from './utils';
import { Templatizer } from './template';

interface TypeName {
    access?: string,
    validationType: string,
}

interface ScriptClassProperty {
    name: string;
    static: boolean;
    readonly: boolean;
    writeonly: boolean;
    type: TypeName;
    get?: boolean;
    set?: boolean;
}

interface NameReference {
    name: string;
    file: string;
}

interface ScriptClass {
    namespace: string[];
    file: string;
    name: string;
    parent?: NameReference;
    isInterface: boolean;
    ctor?: ScriptFunction;
    partial: boolean;
    clone: boolean;
    staticProperties: ScriptClassProperty[];
    properties: ScriptClassProperty[];
    staticMethods: ScriptFunction[];
    methods: ScriptFunction[];
}

interface ScriptEnum {
    namespace: string[];
    file: string;
    name: string;
    members: string[];
}

export interface ScriptFunctionArg {
    name: string;
    type: TypeName;
    partial: boolean;
    optional: boolean;
}

export interface FunctionOverload {
    params: ScriptFunctionArg[];
    requiredArgs: number;
    returnType: TypeName;
}

export interface ScriptFunction {
    name: string;
    namespace?: string[];
    overloads: FunctionOverload[],
    static: boolean,
    jsfunc: boolean,
}

export interface ScriptTypes {
    classes: ScriptClass[];
    enums: ScriptEnum[];
    functions: ScriptFunction[];
    hasDeclarations: boolean;
}

const validationTypes: Record<string, string> = {
    string: 'std::string',
    number: 'double',
    boolean: 'bool',
    void: 'void',
    any: 'JSValue',
    object: 'JSValue',
    ArrayBuffer: 'JSValue',
    StringView: 'std::string_view',
    int: 'int',
    float: 'float',
    double: 'double',
    int8_t: 'int8_t',
    int16_t: 'int16_t',
    int32_t: 'int32_t',
    int64_t: 'int64_t',
    uint8_t: 'uint8_t',
    uint16_t: 'uint16_t',
    uint32_t: 'uint32_t',
    uint64_t: 'uint64_t',
    size_t: 'size_t',
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

    parseType(node: ts.TypeNode, access?: string): TypeName {
        if (ts.isArrayTypeNode(node)) {
            const elementType = this.parseType(node.elementType, access ? `${access}::value_type` : undefined);
            return { access, validationType: `std::vector<${elementType.validationType}>` };
        } else if (ts.isFunctionTypeNode(node)) {
            const rt = this.parseType(node.type).validationType;
            const args = node.parameters.map((x) => {
                const argType = this.parseType(x.type!).validationType;
                if (x.questionToken) {
                    return `std::optional<${argType}>`
                }
                return argType;
            });
            return { access, validationType: `std::function<${rt}(${args.join(',')})>` };
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
            if (node.typeArguments?.length) {
                const base = t;
                switch (base) {
                    case 'SharedPtr': {
                        const inner = this.parseType(node.typeArguments[0]);
                        return { validationType: `std::shared_ptr<${inner.validationType}>` };
                    }
                    case 'UniquePtr': {
                        const inner = this.parseType(node.typeArguments[0]);
                        return { validationType: `std::unique_ptr<${inner.validationType}>` };
                    }
                    case 'Ptr': {
                        const inner = this.parseType(node.typeArguments[0]);
                        return { validationType: `${inner.validationType} *` };
                    }
                    case 'Const': {
                        const inner = this.parseType(node.typeArguments[0]);
                        return { validationType: `const ${inner.validationType}` };
                    }
                    case 'Ref': {
                        const inner = this.parseType(node.typeArguments[0]);
                        return { validationType: `${inner.validationType}&` };
                    }
                    case 'Promise': {
                        return { validationType: `krit::Promise` };
                    }
                    default: {
                        throw new TsError(`unknown parameterized type: ${base}`, node);
                    }
                }
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

    parseProperty(node: ts.PropertyDeclaration | ts.PropertySignature, access?: string): ScriptClassProperty {
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
            writeonly: false,
            static: isStatic,
        };
    }

    parseFunctionArg(node: ts.ParameterDeclaration, access?: string): ScriptFunctionArg {
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
        const argType = this.parseType(type, access);
        if (node.questionToken && !this.hasTag(node, 'defaultValue')) {
            argType.validationType = `std::optional<${argType.validationType}>`;
        }
        return {
            name: node.name.getText(),
            type: argType,
            partial,
            optional: !!node.questionToken,
        };
    }

    parseMethod(node: ts.FunctionDeclaration | ts.MethodDeclaration | ts.MethodSignature, access?: string, jsFunc: boolean = false): ScriptFunction {
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
            overloads: [{
                requiredArgs: jsFunc ? 0 : node.parameters.reduce((a, b) => a + (b.questionToken ? 0 : 1), 0),
                params: jsFunc ? [] : node.parameters.map((arg, i) =>
                    this.parseFunctionArg(
                        arg,
                        access ? `std::tuple_element<${i}, FunctionInfo<${access}>::ArgTypes>::type` : undefined,
                    ),
                ),
                returnType: this.parseType(node.type, access ? `FunctionInfo<${access}>::ReturnType` : undefined),
            }],
            static: isStatic,
            jsfunc: jsFunc,
        };
    }

    parseFunction(node: ts.FunctionDeclaration | ts.MethodDeclaration, access?: string): ScriptFunction {
        const jsFunc = this.hasTag(node, 'jsfunc');
        const f: ScriptFunction = {
            ...this.parseMethod(node, access, jsFunc),
            namespace: this.getNamespace(),
        };
        return f;
    }

    parseConstructor(node: ts.ConstructorDeclaration, access?: string): ScriptFunction {
        return {
            name: "constructor",
            static: false,
            jsfunc: false,
            overloads: [{
                returnType: { validationType: 'void' },
                requiredArgs: node.parameters.reduce((a, b) => a + (b.questionToken ? 0 : 1), 0),
                params: node.parameters.map((arg, i) =>
                    this.parseFunctionArg(
                        arg,
                        access ? `std::tuple_element<${i}, FunctionInfo<${access}>::ArgTypes>::type` : undefined,
                    ),
                ),
            }]
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
                        const cls: ScriptClass = {
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
                                    } else {
                                        found.writeonly = false;
                                    }
                                } else {
                                    cls.properties.push({ name, type, static: false, [p]: true, readonly: p === 'get', writeonly: p === 'set' });
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
                                    const methods = (method.static ? cls.staticMethods : cls.methods);
                                    let pushed = false;
                                    for (let i = 0; i < methods.length; ++i) {
                                        const m = methods[i];
                                        if (m.name === method.name) {
                                            methods[i].overloads.push(method.overloads[0]);
                                            methods[i].overloads.sort((a, b) => b.params.length - a.params.length);
                                            pushed = true;
                                        }
                                    }
                                    if (!pushed) {
                                        methods.push(method);
                                    }
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
                                    const ctor = this.parseConstructor(
                                        node,
                                        `decltype(&${this.namespacedName(cls)}::create)`,
                                    );
                                    if (cls.ctor) {
                                        cls.ctor.overloads.push(ctor.overloads[0]);
                                        cls.ctor.overloads.sort((a, b) => b.params.length - a.params.length);
                                    } else {
                                        cls.ctor = ctor;
                                    }
                                }
                            }
                        });
                        if (cls.partial && !cls.ctor) {
                            // must have a default constructor
                            cls.ctor = { name: 'constructor', static: false, jsfunc: false, overloads: [] };
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
                        // FIXME: support overloads
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
