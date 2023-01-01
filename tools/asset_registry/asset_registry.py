from glob import glob
import os
from jinja2 import Template
import yaml
from PIL import Image

def pascalCase(path):
    return ''.join(filter(lambda s: s.isalpha() or s.isdigit(), list(''.join(s.title() for s in path.split('/')))))

def run(inputPath, outputDir):
    imagePaths = {}
    images = []
    def getImage(path):
        if path not in imagePaths:
            img = { 'path': path, 'width': 0, 'height': 0, 'sizes': [] }
            images.append(img)
            imagePaths[path] = img
            return img
        return imagePaths[path]

    if os.path.exists(inputPath):
        with open(inputPath) as inputFile:
            spec = yaml.safe_load(inputFile)
        assetRoot = spec.get('root', 'assets')
        for item in spec['patterns']:
            if item.get('type') == 'Image':
                matches = glob(os.path.join(assetRoot, item['pattern']), recursive=True)
                for matchPath in matches:
                    format = os.path.basename(matchPath).split('.')[-1]
                    basePath = os.path.join(os.path.dirname(matchPath), os.path.basename(matchPath).split('.')[0]) + '.' + format
                    extension = os.path.basename(matchPath).split('.')[1:]

                    if extension == [format] or extension == ['4k', format]:
                        size = 2160
                    elif extension[0].isdigit():
                        size = int(extension[0])
                    else:
                        raise Exception("Unrecognized image extension: {}".format(matchPath))

                    base = getImage(basePath)
                    if size == 2160:
                        im = Image.open(matchPath)
                        w, h = im.size
                        base['width'] = w
                        base['height'] = h
                    else:
                        child = getImage(matchPath)
                        im = Image.open(matchPath)
                        w, h = im.size
                        child['width'] = w
                        child['height'] = h
                        base['sizes'].append({ 'size': size, 'path': matchPath })

    for artifact in ('images.yaml',):
        outPath = os.path.join(outputDir, artifact)
        existing = None
        if os.path.exists(outPath):
            with open(outPath, 'r') as existingFile:
                existing = existingFile.read()
        with open(os.path.join(os.path.dirname(__file__), artifact + '.jinja2')) as templateFile:
            template = Template(templateFile.read())
        newContent = template.render(images=images)
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
