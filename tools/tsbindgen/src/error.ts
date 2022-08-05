import * as ts from 'typescript';
import * as c from 'ansi-colors';
import * as path from 'path';

function maybeColor(f: (s: string) => string, s: string) {
    return process.stdout.isTTY ? f(s) : s;
}

function lpad(s: string, n: number) {
    if (s.length < n) {
        return ' '.repeat(n - s.length) + s;
    }
    return s.substring(0, n);
}

export class TsError {
    constructor(public message: string, public node: ts.Node) {}

    toString() {
        let s = [];
        const src = this.node.getSourceFile();
        const { line, character } = src.getLineAndCharacterOfPosition(this.node.getStart());
        const { line: line2, character: character2 } = src.getLineAndCharacterOfPosition(this.node.getEnd());
        s.push(
            maybeColor(
                c.red,
                `==> ${maybeColor(c.bold, src.fileName)}: ${line + 1}:${character + 1}-${line2 + 1}:${
                    character2 + 1
                }\n\n`,
            ) +
                this.message +
                '\n',
        );

        // file snippet
        const lines = src.getFullText().split('\n');
        s.push(maybeColor(c.dim, path.basename(src.fileName)));
        for (let i = line; i <= Math.min(line2, lines.length); ++i) {
            if (i > line + 2 && i < line2 - 2) {
                if (i === line + 3) {
                    s.push(maybeColor(c.dim, '             ... ... ...'));
                }
                continue;
            }
            const l = lines[i];
            s.push(maybeColor(c.dim, lpad('' + (i + 1), 6)) + '  ' + maybeColor(c.cyan, l));
            const startPos = i === line ? character : 0;
            const endPos = i === line2 ? character2 : l.length;
            s.push(' '.repeat(startPos + 8) + maybeColor(c.yellow, '^'.repeat(endPos - startPos)));
        }
        return s.join('\n');
    }
}
