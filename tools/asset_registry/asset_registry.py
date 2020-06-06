from glob import glob
import os
from jinja2 import Template
import yaml
from PIL import Image

def assetId(path):
    return ''.join(filter(lambda s: s.isalpha() or s.isdigit(), list(''.join(s.title() for s in path.split('/')))))

def run(inputPath, outputDir):
    with open(inputPath) as inputFile:
        spec = yaml.safe_load(inputFile)
    assets = []
    mtime = 0
    for item in spec:
        matches = glob(item['pattern'], recursive=True)
        for match in matches:
            mtime = max(mtime, os.path.getmtime(match))
            asset = {k: v for (k, v) in item.items()}
            asset['assetId'] = assetId(match)
            asset['path'] = match
            t = asset['type']
            if t == 'Image':
                # get the image dimensions
                im = Image.open(asset['path'])
                w, h = im.size
                asset['width'] = w
                asset['height'] = h

            assets.append(asset)
    for artifact in ('Assets.h', 'Assets.cpp'):
        outPath = os.path.join(outputDir, artifact)
        if os.path.exists(outPath) and os.path.getmtime(outPath) > mtime:
            print('{} is up to date, skipping'.format(outPath))
            continue
        with open(os.path.join(os.path.dirname(__file__), artifact + '.jinja2')) as templateFile:
            template = Template(templateFile.read())
        with open(outPath, 'w') as outFile:
            outFile.write(template.render(assets=assets))


def main():
    import argparse

    parser = argparse.ArgumentParser(description='asset_registry.py')
    parser.add_argument('--input', nargs='?', default='assets.yaml', help='path to assets file; output files will be saved in this directory')
    parser.add_argument('--output-dir', nargs='?', default='.', help='directory where output files will be stored')

    args = parser.parse_args()

    run(args.input, args.output_dir)

if __name__ == '__main__':
    main()
