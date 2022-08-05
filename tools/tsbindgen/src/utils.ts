import * as path from 'path';

export interface ScriptNamespace {
    path: string;
    namespace: string[];
}

export interface BindgenOptions {
    srcDir: string;
    outDir: string;
    filePaths: string[];
    namespace: ScriptNamespace[];
}

export function pickNamespace(options: BindgenOptions, file: string) {
    const f = path.resolve(file);
    for (const namespace of options.namespace) {
        if (f.startsWith(namespace.path)) {
            return namespace.namespace;
        }
    }
    throw new Error(
        `couldn't determine the right native namespace for file at '${file}'; options: ${options.namespace
            .map((x) => x.path)
            .join(', ')}`,
    );
}

export function namespacedName(options: BindgenOptions, o: { file: string; name: string }) {
    const ns = pickNamespace(options, o.file);
    return ns.join('::') + '::' + o.name;
}

export function typename(o: { validationType: string; access?: string }) {
    return o.access ?? o.validationType;
}

export function jsFuncName(o: { namespace: string[]; name: string }) {
    return [...o.namespace, o.name].join('_');
}
