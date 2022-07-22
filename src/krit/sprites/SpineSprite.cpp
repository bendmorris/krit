#include "krit/sprites/SpineSprite.h"
#include "krit/App.h"
#include "krit/UpdateContext.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/math/ScaleFactor.h"
#include "krit/math/Triangle.h"
#include "krit/render/BlendMode.h"
#include "krit/render/DrawKey.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Color.h"
#include "krit/utils/Log.h"
#include "krit/utils/Panic.h"
#include "spine/AnimationState.h"
#include "spine/Attachment.h"
#include "spine/Bone.h"
#include "spine/Color.h"
#include "spine/Extension.h"
#include "spine/MeshAttachment.h"
#include "spine/RTTI.h"
#include "spine/RegionAttachment.h"
#include "spine/Slot.h"
#include "spine/Vector.h"

struct KritSpineExtension : public spine::DefaultSpineExtension {
    char *_readFile(const spine::String &path, int *length) override {
        return krit::IoRead::read(std::string(path.buffer()), length);
    }

    void *_alloc(size_t size, const char *_1, int _2) override {
        if (!size) {
            return nullptr;
        }
        return krit::IoRead::alloc(size);
    }

    void *_calloc(size_t size, const char *file, int line) override {
        if (!size) {
            return nullptr;
        }
        return krit::IoRead::alloc(size);
    }

    void *_realloc(void *ptr, size_t size, const char *file,
                   int line) override {
        return krit::IoRead::realloc(ptr, size);
    }

    void _free(void *mem, const char *_1, int _2) override {
        krit::IoRead::free(mem);
    }
};

spine::SpineExtension *spine::getDefaultExtension() {
    return new KritSpineExtension();
}

namespace krit {

SpineTextureLoader SpineTextureLoader::instance;

std::string SpineSprite::defaultAtlasPath;

template <>
std::shared_ptr<SpineData>
AssetLoader<SpineData>::loadAsset(const std::string &path) {
    auto data = std::make_shared<SpineData>();
    const spine::String &atlasName(
        SpineSprite::defaultAtlasPath.empty()
            ? (path.substr(0, path.length() - 5) + ".atlas").c_str()
            : SpineSprite::defaultAtlasPath.c_str());
    data->atlas = std::unique_ptr<spine::Atlas>(
        new spine::Atlas(atlasName, &SpineTextureLoader::instance));
    data->binary = std::unique_ptr<spine::SkeletonBinary>(
        new spine::SkeletonBinary(data->atlas.get()));
    // data->json = std::unique_ptr<spine::SkeletonJson>(
    //     new spine::SkeletonJson(data->atlas.get()));
    int length;
    unsigned char *s = (unsigned char *)IoRead::read(path, &length);
    data->skeletonData = std::unique_ptr<spine::SkeletonData>(
        data->binary->readSkeletonData(s, length));
    // data->skeletonData = std::unique_ptr<spine::SkeletonData>(
    //     data->json->readSkeletonData((const char *)s));
    if (!data->skeletonData) {
        Log::error("failed to load skeleton data: %s", path.c_str());
    }
    data->animationStateData = std::unique_ptr<spine::AnimationStateData>(
        new spine::AnimationStateData(data->skeletonData.get()));
    IoRead::free((char *)s);
    return data;
}

template <> size_t AssetLoader<SpineData>::cost(SpineData *s) {
    return sizeof(SpineData) + sizeof(spine::Atlas) +
           sizeof(spine::SkeletonBinary) + sizeof(spine::SkeletonData) +
           sizeof(spine::AnimationStateData);
}

template <> AssetType AssetLoader<SpineData>::type() {
    return SpineSkeletonAsset;
}

void SpineTextureLoader::load(spine::AtlasPage &page,
                              const spine::String &path) {
    std::string assetName(path.buffer());
    std::shared_ptr<ImageData> texture = App::ctx.engine->getImage(assetName);
    ImageRegion *region = new ImageRegion(texture);
    page.setRendererObject(region);
    page.width = texture->width();
    page.height = texture->height();
}

void SpineTextureLoader::unload(void *texture) {
    ImageRegion *region = static_cast<ImageRegion *>(texture);
    delete region;
}

SpineSprite::SpineSprite(const std::string &id) {
    spine::Bone::setYDown(true);

    // load skeleton/animation data
    this->smooth = SmoothMipmap;
    this->data = App::ctx.engine->getSpine(id);
    this->skeleton = std::unique_ptr<spine::Skeleton>(
        new spine::Skeleton(&this->skeletonData()));
    this->animationState = std::unique_ptr<spine::AnimationState>(
        new spine::AnimationState(&this->animationStateData()));
    this->skin =
        std::unique_ptr<spine::Skin>(new spine::Skin(spine::String("custom")));

    this->skeleton->update(0);
    this->animationState->update(0);
    this->animationState->apply(*this->skeleton);
    this->skeleton->updateWorldTransform();
}

float SpineSprite::worldVertices[1024] = {0};

float SpineSprite::setAnimation(size_t track, const std::string &name,
                                bool loop, float speed, float mix) {
    // printf("set animation: %s\n", name.c_str());
    auto trackEntry = this->animationState->setAnimation(
        track, spine::String(name.c_str()), loop);
    if (speed != 1) {
        trackEntry->setTimeScale(speed);
    }
    if (mix > 0) {
        trackEntry->setMixDuration(
            std::min(trackEntry->getAnimationEnd(), mix));
    }
    return std::max(1.0f / 60, trackEntry->getAnimationEnd());
}

float SpineSprite::addAnimation(size_t track, const std::string &name,
                                bool loop, float delay, float mix) {
    auto trackEntry = this->animationState->addAnimation(
        track, spine::String(name.c_str()), loop, delay);
    if (mix > 0) {
        trackEntry->setMixDuration(
            std::min(trackEntry->getAnimationEnd(), mix));
    }
    return std::max(1.0f / 60, trackEntry->getAnimationEnd());
}

const char *SpineSprite::getAnimation(size_t track) {
    spine::TrackEntry *t = animationState->getCurrent(track);
    if (t) {
        return t->getAnimation()->getName().buffer();
    }
    return "";
}

spine::String SpineSprite::_customSkin("custom");

void SpineSprite::advance(float t) {
    if (t) {
        this->skeleton->update(t);
        this->animationState->update(t);
        this->animationState->apply(*this->skeleton);
        this->skeleton->updateWorldTransform();
    }
}

void SpineSprite::setSkin(const std::string &name) {
    spine::Skin *newSkin =
        this->skeletonData().findSkin(spine::String(name.c_str()));
    if (newSkin) {
        this->skin->addSkin(newSkin);
    } else {
        panic("unknown skin: %s\n", name.c_str());
    }
    this->skeleton->setSkin(this->skin.get());
    this->skeleton->setSlotsToSetupPose();
    this->animationState->apply(*this->skeleton);
}

void SpineSprite::removeSkin(const std::string &name) {
    spine::Skin *oldSkin =
        this->skeletonData().findSkin(spine::String(name.c_str()));
    spine::Skin::AttachmentMap::Entries entries = oldSkin->getAttachments();
    while (entries.hasNext()) {
        spine::Skin::AttachmentMap::Entry &entry = entries.next();
        skin->removeAttachment(entry._slotIndex, entry._name);
    }
}

void SpineSprite::update(UpdateContext &ctx) {
    animationState->setTimeScale(this->rate);
    float elapsed = ctx.elapsed;
    if (elapsed > 0) {
        this->advance(elapsed);
    }
}

void SpineSprite::render(RenderContext &ctx) {
    spine::Skeleton &skeleton = *this->skeleton;
    spine::Vector<spine::Slot *> &drawOrder = skeleton.getDrawOrder();
    size_t size = drawOrder.size();

    Matrix4 m;
    m.identity();
    m.scale(scale.x(), scale.y());
    m.rotate(angle);
    m.pitch(pitch);
    m.translate(position.x(), position.y(), position.z());

    for (size_t i = 0; i < size; ++i) {
        spine::Slot *slot = drawOrder[i];
        Color color;
        color.r = this->color.r * skeleton.getColor().r * slot->getColor().r;
        color.g = this->color.g * skeleton.getColor().g * slot->getColor().g;
        color.b = this->color.b * skeleton.getColor().b * slot->getColor().b;
        color.a = this->color.a * skeleton.getColor().a * slot->getColor().a;
        spine::Attachment *attachment = slot->getAttachment();
        if (!attachment) {
            continue;
        }
        BlendMode blendMode = Alpha;
        // switch (slot->getData().getBlendMode()) {
        //     case spine::BlendMode_Normal: {
        //         blendMode = Add;
        //         break;
        //     }
        //     case spine::BlendMode_Multiply: {
        //         blendMode = Multiply;
        //         break;
        //     }
        //     case spine::BlendMode_Screen: {
        //         blendMode = BlendScreen;
        //         break;
        //     }
        //     default: {
        //         blendMode = Alpha;
        //     }
        // }
        // var tintR: Float = skeleton.r * slot.r;
        // var tintG: Float = skeleton.g * slot.g;
        // var tintB: Float = skeleton.b * slot.b;
        // var tintA: Float = skeleton.a * slot.a;
        if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
            float *worldVertices = SpineSprite::worldVertices;
            spine::RegionAttachment *regionAttachment =
                static_cast<spine::RegionAttachment *>(attachment);
            auto overridden = customAttachments.find(attachment);
            ImageRegion *region;
            Triangle uvt1;
            Triangle uvt2;
            if (overridden == customAttachments.end()) {
                region = static_cast<ImageRegion *>(
                    (static_cast<spine::AtlasRegion *>(
                         regionAttachment->getRendererObject()))
                        ->page->getRendererObject());
                spine::Vector<float> &uvs = regionAttachment->getUVs();
                uvt1.p1.setTo(uvs[0], uvs[1]);
                uvt1.p2.setTo(uvs[2], uvs[3]);
                uvt1.p3.setTo(uvs[4], uvs[5]);
                uvt2.p1.setTo(uvs[4], uvs[5]);
                uvt2.p2.setTo(uvs[6], uvs[7]);
                uvt2.p3.setTo(uvs[0], uvs[1]);
            } else {
                region = overridden->second.get();
                auto &imageData = region->img;
                auto &rect = region->rect;
                float uvx1 = rect.x / static_cast<float>(imageData->width());
                float uvy1 = rect.y / static_cast<float>(imageData->height());
                float uvx2 = (rect.x + rect.width) / static_cast<float>(imageData->width());
                float uvy2 = (rect.y + rect.height) / static_cast<float>(imageData->height());
                uvt2.p1.setTo(uvx1, uvy1);
                uvt2.p2.setTo(uvx2, uvy1);
                uvt2.p3.setTo(uvx2, uvy2);
                uvt1.p1.setTo(uvx2, uvy2);
                uvt1.p2.setTo(uvx1, uvy2);
                uvt1.p3.setTo(uvx1, uvy1);
            }
            regionAttachment->computeWorldVertices(slot->getBone(),
                                                   worldVertices, 0, 2);
            DrawKey key;
            key.shader = this->shader;
            key.image = region->img;
            key.smooth = this->smooth;
            key.blend = blendMode;
            Triangle t1(worldVertices[0], worldVertices[1], worldVertices[2],
                        worldVertices[3], worldVertices[4], worldVertices[5]);
            Triangle t2(worldVertices[4], worldVertices[5], worldVertices[6],
                        worldVertices[7], worldVertices[0], worldVertices[1]);
            
            t1 = m * t1;
            t2 = m * t2;
            ctx.addTriangle(key, t1, uvt1, color);
            ctx.addTriangle(key, t2, uvt2, color);
        } else if (attachment->getRTTI().isExactly(
                       spine::MeshAttachment::rtti)) {
            float *worldVertices = SpineSprite::worldVertices;
            spine::MeshAttachment *meshAttachment =
                static_cast<spine::MeshAttachment *>(attachment);
            ImageRegion *region = static_cast<ImageRegion *>(
                (static_cast<spine::AtlasRegion *>(
                     meshAttachment->getRendererObject()))
                    ->page->getRendererObject());
            // before rendering via spSkeleton_updateWorldTransform
            meshAttachment->computeWorldVertices(*slot, worldVertices);
            DrawKey key;
            key.shader = this->shader;
            key.image = region->img;
            key.smooth = this->smooth;
            key.blend = blendMode;
            float *uvs = meshAttachment->getUVs().buffer();
            auto &triangles = meshAttachment->getTriangles();
            for (size_t i = 0; i < triangles.size() / 3; ++i) {
                int i0 = triangles[i * 3] << 1;
                int i1 = triangles[i * 3 + 1] << 1;
                int i2 = triangles[i * 3 + 2] << 1;
                Triangle t(worldVertices[i0], worldVertices[i0 + 1],
                           worldVertices[i1], worldVertices[i1 + 1],
                           worldVertices[i2], worldVertices[i2 + 1]);
                Triangle uvt(uvs[i0], uvs[i0 + 1], uvs[i1], uvs[i1 + 1],
                             uvs[i2], uvs[i2 + 1]);
                t = m * t;
                ctx.addTriangle(key, t, uvt, color);
            }
        }
    }
}

}
