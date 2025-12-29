import * as fs from 'fs';
import * as path from 'path';
import * as nunjucks from 'nunjucks';
import { BindgenOptions, jsFuncName, namespacedName, pickNamespace, typename } from './utils';
import { ScriptTypes } from './bindgen';

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
        function call(prefix, params) {
            return `${prefix}(${params
                .map((param, i) =>
                    param.partial
                        ? `ScriptValueFromPartial<${typename(param.type)}>::valueFromPartial(ctx, argv[${i}])`
                        : `TypeConverter<${typename(param.type)}>::valueFromJs(ctx, argv[${i}])`,
                )
                .join(', ')})`;
        }
        env.addFilter('call', function (prefix, method) {
            let s = '';
            if (method.optionalArgs > 0) {
                for (let i = 0; i < method.optionalArgs; ++i) {
                    s += `(argc < ${method.params.length - method.optionalArgs + i + 1}) ? (${call(
                        prefix,
                        method.params.slice(0, method.params.length - method.optionalArgs + i),
                    )}) : `;
                }
            }
            return s + call(prefix, method.params);
        });
        env.addGlobal('namespacedName', (o: { file: string; name: string }) => namespacedName(this.options, o));

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
        } catch (_) {}
        this.replaceIfDifferent(
            outPath,
            this.env.render('templates/ScriptClass.cpp.njk', {
                includeHeader: path.join(this.options.srcDir, filename.replace('.d.ts', '.h')),
                namespace: pickNamespace(this.options, this.options.srcDir).join('::'),
                types,
            }),
        );
    }
}
