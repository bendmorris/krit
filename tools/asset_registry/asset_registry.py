from glob import glob
import os
from jinja2 import Template
import yaml
from PIL import Image

def pascalCase(path):
    return ''.join(filter(lambda s: s.isalpha() or s.isdigit(), list(''.join(s.title() for s in path.split('/')))))

def run(inputPath, outputDir):
    nextAssetId = 0
    assetIds = {}
    if os.path.exists(inputPath):
        with open(inputPath) as inputFile:
            spec = yaml.safe_load(inputFile)
        assetRoot = spec.get('root', 'assets')
        variants = spec.get('variants', [])

        assets = []
        mtime = 0
        roots = [{'id': 'AssetRootBase', 'path': assetRoot, 'assets': [], 'resolution': 2160 }]
        roots.extend([{'id': 'AssetRoot' + pascalCase(str(v.get('path', ''))), '_id': v.get('path'), 'resolution': v.get('resolution'), 'path': os.path.join(assetRoot, str(v.get('path', ''))), 'scale': v.get('scale', 1), 'assets': []} for v in variants])

        for item in spec['patterns']:
            for n, root in enumerate(roots):
                matches = glob(os.path.join(root['path'], item['pattern']), recursive=True)
                for matchPath in matches:
                    match = matchPath[len(root['path'])+1:]
                    assetId = assetIds.get(match)
                    name = pascalCase(match)
                    if assetId is None:
                        assetId = nextAssetId
                        nextAssetId += 1
                        assetIds[match] = assetId
                        for _root in roots:
                            _root['assets'].append(None)
                        assets.append({ 'id': name, 'path': match })
                    mtime = max(mtime, os.path.getmtime(matchPath))
                    asset = {k: v for (k, v) in item.items()}
                    asset['id'] = name
                    asset['path'] = matchPath
                    asset['lookupPath1'] = match
                    asset['lookupPath2'] = os.path.join(assetRoot, match)
                    asset['paths'] = [matchPath, match, os.path.join(assetRoot, match)]
                    t = asset['type']
                    if t == 'Image':
                        # get the image dimensions
                        im = Image.open(matchPath)
                        w, h = im.size
                        asset['width'] = asset['realWidth'] = w
                        asset['height'] = asset['realHeight'] = h
                        scale = root.get('resolution') / 2160.0
                        asset['width'] /= scale
                        asset['height'] /= scale
                        asset['scale'] = scale
                    root['assets'][assetId] = asset
    else:
        assets = []
        roots = {}
    for artifact in ('assets.yaml',):
        outPath = os.path.join(outputDir, artifact)
        existing = None
        if os.path.exists(outPath):
            with open(outPath, 'r') as existingFile:
                existing = existingFile.read()
        with open(os.path.join(os.path.dirname(__file__), artifact + '.jinja2')) as templateFile:
            template = Template(templateFile.read())
        newContent = template.render(assets=assets, roots=roots)
        if not existing or newContent != existing:
            with open(outPath, 'w') as outFile:
                outFile.write(newContent)
        else:
            print('no change to {}; skipping'.format(artifact))


def main():
    import argparse

    parser = argparse.ArgumentParser(description='asset_registry.py')
    parser.add_argument('--input', nargs='?', default='assets.yaml', help='path to assets file; output files will be saved in this directory')
    parser.add_argument('--output-dir', nargs='?', default='.', help='directory where output files will be stored')

    args = parser.parse_args()

    run(args.input, args.output_dir)

if __name__ == '__main__':
    main()
