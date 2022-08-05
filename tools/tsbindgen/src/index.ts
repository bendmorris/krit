import * as fs from 'fs';
import * as path from 'path';
import { Command } from 'commander';
import { Bindgen } from './bindgen';
import { BindgenOptions } from './utils';

function main() {
    const startTime = Date.now();

    const cli = new Command();
    cli.name('krit-tsbindgen')
        .description('Binding generator for Krit')
        // .version(pkg.version)
        .requiredOption('-o, --out-dir <path>', 'output directory for generated files')
        .requiredOption('-s, --src-dir <path>', 'source directory')
        .option(
            '-n, --namespace <a.b.c>',
            'default top-level namespace when not in a namespace declaration',
            (val, prev) => {
                const [namespace, root] = val.split(':');
                prev.push({ namespace: namespace.split('.'), path: path.resolve(root) });
                return prev;
            },
            [],
        );

    cli.parse();
    const opts: BindgenOptions = cli.opts();

    opts.srcDir = path.resolve(opts.srcDir);

    opts.filePaths = (function walk(dir) {
        var results: string[] = [];
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
    })(opts.srcDir);

    const bindgen = new Bindgen(opts);
    bindgen.execute();

    console.log(`krit-tsbindgen finished, duration: ${Date.now() - startTime}ms`);
}

main();
