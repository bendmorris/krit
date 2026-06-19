import * as fs from 'fs';
import * as path from 'path';
import * as nunjucks from 'nunjucks';
import { BindgenOptions, jsFuncName, namespacedName, pickNamespace, typename } from './utils';
import { FunctionOverload, ScriptFunction, ScriptFunctionArg, ScriptTypes } from './bindgen';

export class Templatizer {
    env: nunjucks.Environment;

    constructor(public options: BindgenOptions) {
        const env = (this.env = nunjucks.configure(path.join(__dirname, '../'), {
            autoescape: false,
            trimBlocks: true,
            lstripBlocks: true,
        }));
        env.addFilter('repeat', function (str, count) {
            return str.repeat(count || 0);
        });
        env.addFilter('escapeName', function (str) {
            return str.replace('$', '__dollar__');
        });
        env.addFilter('isArray', function (obj) {
            return Array.isArray(obj);
        });
        function call(prefix, params: ScriptFunctionArg[]) {
            return `${prefix}(${params
                .map((param, i) =>
                    param.partial
                        ? `ScriptValueFromPartial<${typename(param.type)}>::valueFromPartial(ctx, argv[${i}])`
                        : `TypeConverter<${typename(param.type)}>::valueFromJs(ctx, argv[${i}])`,
                )
                .join(', ')})`;
        }
        env.addFilter('call', function (prefix, overload: FunctionOverload) {
            let s = '';
            const optionalArgs = overload.params.length - overload.requiredArgs;
            if (overload.requiredArgs < overload.params.length) {
                for (let i = 0; i < optionalArgs; ++i) {
                    s += `(argc < ${overload.requiredArgs + i + 1}) ? (${call(
                        prefix,
                        overload.params.slice(0, overload.requiredArgs + i),
                    )}) : `;
                }
            }
            return s + call(prefix, overload.params);
        });
        env.addFilter('title', function (s: string) {
            return s.charAt(0).toUpperCase() + s.substring(1);
        });
        env.addGlobal('namespacedName', (o: { file: string; name: string }) => namespacedName(this.options, o));
        env.addGlobal('paramLength', function (s: ScriptFunction) {
            return s.overloads.reduce((a, b) => Math.max(a, b.params.length), 0);
        });

        env.addGlobal('namespaceArray', ({ namespace }: { namespace: string[] }) => {
            return `{ ${[...(namespace ?? []).map((x) => `"${x}"`), 'nullptr']} }`;
        });
        env.addGlobal('typename', typename);
        env.addGlobal('jsFuncName', jsFuncName);
        env.addGlobal('JSON', JSON);
    }

    replaceIfDifferent(path: string, content: string) {
        if (fs.existsSync(path)) {
            if (fs.readFileSync(path).toString() === content) {
                // console.log(`skipping ${path} which hasn't changed`);
                return;
            }
        }
        console.log(`updating ${path}...`);
        fs.writeFileSync(path, content);
    }

    generateTemplates(filename: string, types: ScriptTypes) {
        const outPath = path.join(this.options.outDir, `${filename}.ScriptClass.cpp`);
        try {
            fs.mkdirSync(path.dirname(outPath), { recursive: true });
        } catch (_) { }
        let content: string;
        try {
            content = this.env.render('templates/ScriptClass.cpp.njk', {
                includeHeader: path.join(this.options.srcDir, filename.replace('.d.ts', '.h')),
                namespace: pickNamespace(this.options, this.options.srcDir).join('::'),
                types,
            });
        } catch (e) {
            console.error("error while rendering " + filename);
            throw e;
        }
        this.replaceIfDifferent(
            outPath,
            content,
        );
    }
}
