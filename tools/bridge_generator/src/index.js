const startTime = Date.now();

const ts = require('typescript');
const fs = require('fs');
const path = require('path');
const nunjucks = require('nunjucks');
const { SyntaxKind } = require('typescript');

const typeMap = new Map(
    Object.entries({
        void: undefined,
        boolean: { type: 'bool', pointer: 0 },
        number: { type: 'double', pointer: 0 },
        integer: { type: 'int', pointer: 0 },
        float: { type: 'float', pointer: 0 },
        size_t: { type: 'size_t', pointer: 0 },
        cstring: { type: 'const char', pointer: 1 },
        string_view: { type: 'std::string_view', pointer: 0 },
        string: { type: 'std::string', pointer: 0 },
        any: { type: 'JSValue', pointer: 0 },
    }),
);

const [_, __, bridgePath, outputDir] = process.argv;

if (!outputDir) {
    console.log('usage: node bridge_generator.js <path> <outputDir>');
    process.exit(1);
}

const basePaths = ['../../script'];
if (bridgePath) {
    basePaths.push(bridgePath);
}

const filePaths = [].concat(
    ...basePaths.map(function walk(dir) {
        var results = [];
        var list = fs.readdirSync(dir);
        list.forEach(function (file) {
            if (file === 'node_modules') {
                return;
            }
            file = path.join(dir, file);
            var stat = fs.statSync(file);
            if (stat && stat.isDirectory()) {
                /* Recurse into a subdirectory */
                results = results.concat(walk(file));
            } else {
                /* Is a file */
                if (file.endsWith('.d.ts')) {
                    results.push(file);
                }
            }
        });
        return results;
    }),
);

const program = ts.createProgram(filePaths, {
    target: 'es2020',
    typeRoots: basePaths,
    lib: ['lib.es2020.d.ts'],
    moduleResolution: 'node',
});
let checker = program.getTypeChecker();

function cppType(t) {
    if (t.intrinsicName === 'error') {
        throw new Error('unrecognized type');
    }
    // const enums are integers
    if (t.getSymbol() && t.getSymbol().valueDeclaration && ts.isEnumDeclaration(t.getSymbol().valueDeclaration)) {
        return { type: 'int', pointer: 0, cast: true };
    }
    const s = checker.typeToString(t);
    // arrays are vectors
    if (checker.isArrayType(t)) {
        const subType = cppType(t.resolvedTypeArguments[0]);
        return { type: `std::vector<${subType.type}${'*'.repeat(subType.pointer)}>`, pointer: 0 };
    }
    // known builtin types
    if (typeMap.has(s)) {
        return typeMap.get(s);
    }
    switch (t.aliasSymbol ? t.aliasSymbol.name : undefined) {
        case 'Reference': {
            const subType = cppType(t.aliasTypeArguments[0]);
            return { ...subType, reference: true };
        }
        case 'Pointer': {
            const { type, pointer } = cppType(t.aliasTypeArguments[0]);
            return { type, pointer: pointer + 1 };
        }
        case 'StringMap': {
            const subType = cppType(t.aliasTypeArguments[0]);
            return { type: `std::unordered_map<std::string, ${subType.type}>&`, pointer: false };
        }
    }
    // unrecognized, treat as custom type
    return { type: s, pointer: 0 };
}

function tagify(v) {
    if (v === undefined) {
        return true;
    }
    try {
        return JSON.parse(v);
    } catch (_) {
        return v;
    }
}

const functions = [];
const wrappers = [];
const enums = [];

function defineFunction(node, functionType, namespace = []) {
    const name = node.name.text;
    const returnType = functionType.getReturnType();
    const type = cppType(returnType);
    const func = {
        name,
        namespace: 'krit',
        fullName: namespace.concat(name).join('_'),
        bridgeNamespace: namespace,
        returnCppType: type,
        params: [],
        tags: {},
    };
    for (const tag of ts.getJSDocTags(node)) {
        if (tag.tagName.text === 'namespace') {
            func.namespace = tag.comment;
        } else {
            func.tags[tag.tagName.text] = tagify(tag.comment);
        }
    }
    for (const param of functionType.getParameters()) {
        const type = cppType(checker.getTypeOfSymbolAtLocation(param, node));
        func.params.push({
            name: param.name,
            cppType: type,
        });
    }
    functions.push(func);
}

for (const sourceFile of program.getSourceFiles()) {
    if (sourceFile.path.indexOf('node_modules') !== -1) {
        continue;
    }
    ts.forEachChild(sourceFile, function visit(node) {
        if (ts.isVariableStatement(node)) {
            let isNamespace = false;
            if (node.jsDoc) {
                for (const doc of node.jsDoc) {
                    for (const tag of doc.tags) {
                        if (tag.tagName.text === 'namespace') {
                            isNamespace = true;
                            break;
                        }
                    }
                }
            }
            if (isNamespace) {
                // this node defines a namespace
                for (const decl of node.declarationList.declarations) {
                    const namespace = decl.name.text;
                    const type = checker.getTypeAtLocation(decl);
                    for (const prop of checker.getPropertiesOfType(type)) {
                        // for each prop, define either a function or a sub-namespace
                        // TODO: support nested sub-namespaces
                        const propType = checker.getTypeOfSymbolAtLocation(prop, decl).getCallSignatures()[0];
                        defineFunction(prop.valueDeclaration, propType, [namespace]);
                    }
                }
            }
        } else if (ts.isFunctionDeclaration(node)) {
            if (!node.jsDoc) {
                return;
            }
            // this node defines a function
            const functionType = checker.getTypeAtLocation(node).getCallSignatures()[0];
            defineFunction(node, functionType);
        } else if (ts.isClassDeclaration(node) || ts.isInterfaceDeclaration(node)) {
            if (!node.jsDoc) {
                return;
            }
            const options = {
                namespace: 'krit',
                name: node.name.text,
                props: [],
                methods: [],
                staticProps: [],
                staticMethods: [],
                import: [],
                constructor: undefined,
                from: false,
            };
            for (const doc of node.jsDoc) {
                for (const tag of doc.tags) {
                    const tagName = tag.tagName.text;
                    if (options[tagName] && Array.isArray(options[tagName])) {
                        options[tagName].push(tag.comment);
                    } else {
                        options[tagName] = tagify(tag.comment);
                    }
                }
            }
            const type = checker.getTypeAtLocation(node);
            if (type.symbol.exports && type.symbol.exports.has('from')) {
                options.from = true;
            }
            if (ts.isClassDeclaration(node) && type.symbol.members && type.symbol.members.has('__constructor')) {
                const symbol = checker.getSymbolAtLocation(node.name);
                const constructorType = checker.getTypeOfSymbolAtLocation(symbol, symbol.valueDeclaration);
                if (type.symbol.members.get('__constructor').declarations.length > 1) {
                    throw new Error(
                        `type ${node.name.text} has multiple constructors; only one constructor is supported`,
                    );
                }
                const functionType = constructorType.getConstructSignatures()[0];
                options.constructor = { params: [] };
                for (const param of functionType.getParameters()) {
                    const spec = {
                        name: param.name,
                        cppType: cppType(checker.getTypeOfSymbolAtLocation(param, node)),
                        tags: {},
                    };
                    for (const tag of ts.getJSDocTags(param.valueDeclaration)) {
                        spec.tags[tag.tagName.text] = tagify(tag.comment);
                    }
                    options.constructor.params.push(spec);
                }
            }
            const staticProperties = checker
                .getPropertiesOfType(checker.getTypeOfSymbolAtLocation(node.symbol, node))
                .filter((x) => ['prototype', 'from'].indexOf(x.escapedName) === -1);
            const properties = checker.getPropertiesOfType(type);
            for (const collection of [
                { methods: options.staticMethods, props: options.staticProps, defined: staticProperties },
                { methods: options.methods, props: options.props, defined: properties },
            ]) {
                for (const prop of collection.defined) {
                    const propType = checker.getTypeOfSymbolAtLocation(prop, node);
                    const tags = {};
                    for (tag of ts.getJSDocTags(prop.valueDeclaration)) {
                        tags[tag.tagName.text] = tagify(tag.comment);
                    }
                    if (tags.skip) {
                        continue;
                    }
                    if (checker.typeToString(propType).indexOf('=>') !== -1) {
                        // this is a method
                        const functionType = propType.getCallSignatures()[0];
                        const method = {
                            name: prop.name,
                            returnCppType: cppType(functionType.getReturnType()),
                            params: [],
                            tags,
                        };
                        for (const param of functionType.getParameters()) {
                            const spec = {
                                name: param.name,
                                cppType: cppType(checker.getTypeOfSymbolAtLocation(param, node)),
                                tags: {},
                            };
                            for (const tag of ts.getJSDocTags(param.valueDeclaration)) {
                                spec.tags[tag.tagName.text] = tagify(tag.comment);
                            }
                            method.params.push(spec);
                        }
                        collection.methods.push(method);
                    } else {
                        // this is a property
                        let type = cppType(propType);
                        if (tags.cppType) {
                            type = { type: tags.cppType };
                        }
                        collection.props.push({
                            name: prop.name,
                            cppType: type,
                            tags,
                        });
                    }
                }
            }
            wrappers.push(options);
        } else if (ts.isModuleDeclaration(node)) {
            ts.forEachChild(node, visit);
        } else if (ts.isEnumDeclaration(node)) {
            if (!node.jsDoc) {
                return;
            }
            const isConst = (node.modifiers || []).reduce(
                (prev, next) => prev || next.kind === SyntaxKind.ConstKeyword,
                false,
            );
            if (!isConst) {
                const e = {
                    namespace: 'krit',
                    prefix: '',
                    name: node.name.text,
                    members: node.members.map((member) => member.name.text),
                    import: [],
                };
                for (const doc of node.jsDoc) {
                    for (const tag of doc.tags) {
                        const tagName = tag.tagName.text;
                        if (e[tagName] && Array.isArray(e[tagName])) {
                            e[tagName].push(tag.comment);
                        } else {
                            e[tagName] = tagify(tag.comment);
                        }
                    }
                }
                enums.push(e);
            }
        }
    });
}

const bridges = { namespaces: [], functions: [] };
for (const f of functions) {
    let bridge = bridges;
    for (const name of f.bridgeNamespace) {
        let existing;
        for (const namespace of bridge.namespaces) {
            if (namespace.bridgeNamespace === name) {
                existing = namespace;
                break;
            }
        }
        if (!existing) {
            bridge.namespaces.push(
                (existing = {
                    bridgeNamespace: name,
                    functions: [],
                    namespaces: [],
                }),
            );
        }
        bridge = existing;
    }
    bridge.functions.push(f);
}

const scriptDir = path.join(outputDir, 'script');
if (!fs.existsSync(scriptDir)) {
    fs.mkdirSync(scriptDir, { recursive: true });
}

var env = nunjucks.configure({ autoescape: false });
env.addFilter('repeat', function (str, count) {
    return str.repeat(count || 0);
});
env.addFilter('escapeName', function (str) {
    return str.replace('$', '__dollar__');
});

function replaceIfDifferent(path, content) {
    if (fs.existsSync(path)) {
        if (fs.readFileSync(path).toString() === content) {
            // console.log(`skipping ${path} which hasn't changed`);
            return;
        }
    }
    console.log(`generating ${path}...`);
    fs.writeFileSync(path, content);
}

// generate native function declarations
replaceIfDifferent(
    path.join(scriptDir, 'ScriptBridge.h'),
    env.render('templates/ScriptBridge.h.nj', {
        bridgeFuncs: functions,
    }),
);

// generate script engine init function implementation to define all bridges
replaceIfDifferent(
    path.join(scriptDir, 'ScriptBridge.cpp'),
    env.render('templates/ScriptBridge.cpp.nj', {
        bridgeFuncs: functions,
        bridges,
    }),
);

// generate ScriptClass declaration
replaceIfDifferent(
    path.join(scriptDir, 'ScriptClass.h'),
    env.render('templates/ScriptClass.h.nj', {
        wrappers,
    }),
);

replaceIfDifferent(
    path.join(scriptDir, 'ScriptClass.cpp'),
    env.render('templates/ScriptClass.cpp.nj', {
        enums,
    }),
);

// generate script classes

for (const wrapper of wrappers) {
    replaceIfDifferent(
        path.join(scriptDir, `ScriptClass.${wrapper.namespace.replace('::', '.')}.${wrapper.name}.cpp`),
        env.render('templates/ScriptClass.class.cpp.nj', {
            wrapper,
        }),
    );
}

console.log(`bridge generation duration: ${Date.now() - startTime}ms`);
