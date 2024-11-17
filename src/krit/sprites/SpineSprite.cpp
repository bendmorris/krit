#include "krit/sprites/SpineSprite.h"
#include "krit/Engine.h"
#include "krit/UpdateContext.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
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
        std::string s = krit::engine->io->readFile(path.buffer());
        *length = s.size();
        char *c = (char *)malloc(s.size() + 1);
        memcpy(c, s.c_str(), s.size());
        c[s.size()] = 0;
        return c;
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
    std::string s = engine->io->readFile(path.c_str());
    data->skeletonData = std::unique_ptr<spine::SkeletonData>(
        data->binary->readSkeletonData((unsigned char *)s.c_str(), s.size()));
    // data->skeletonData = std::unique_ptr<spine::SkeletonData>(
    //     data->json->readSkeletonData((const char *)s));
    if (!data->skeletonData) {
        Log::error("failed to load skeleton data %s: %s", path.c_str(),
                   data->binary->getError().buffer());
    }
    data->animationStateData = std::unique_ptr<spine::AnimationStateData>(
        new spine::AnimationStateData(data->skeletonData.get()));
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
    std::shared_ptr<ImageData> texture = engine->getImage(assetName);
    ImageRegion *region = new ImageRegion(texture);
    page.texture = region;
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
    this->data = engine->getSpine(id);
    this->skeleton = std::unique_ptr<spine::Skeleton>(
        new spine::Skeleton(&this->skeletonData()));
    this->animationState = std::unique_ptr<spine::AnimationState>(
        new spine::AnimationState(&this->animationStateData()));
    this->skin =
        std::unique_ptr<spine::Skin>(new spine::Skin(spine::String("custom")));

    this->skeleton->update(0);
    this->animationState->update(0);
    this->animationState->apply(*this->skeleton);
    this->skeleton->updateWorldTransform(spine::Physics::Physics_None);
}

float SpineSprite::worldVertices[1024] = {0};

float SpineSprite::setAnimation(size_t track, const std::string &name,
                                bool loop, float speed, float mix) {
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

void SpineSprite::stopAnimation(size_t track) {
    spine::TrackEntry *t = animationState->getCurrent(track);
    if (t) {
        t->setLoop(false);
    }
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

// spine::String SpineSprite::_customSkin("custom");

void SpineSprite::advance(float t) {
    if (t) {
        this->skeleton->update(t);
        this->animationState->update(t);
        this->animationState->apply(*this->skeleton);
        this->skeleton->updateWorldTransform(spine::Physics::Physics_None);
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

void SpineSprite::update() {
    animationState->setTimeScale(this->rate);
    float elapsed = frame.elapsed;
    if ((this->rate * elapsed) > 0) {
        this->advance(elapsed);
    }
}

static spine::SkeletonRenderer skeletonRenderer;

void SpineSprite::render(RenderContext &ctx) {
    spine::Skeleton &skeleton = *this->skeleton;

    Matrix4 m;
    m.identity();
    m.translate(-origin.x(), -origin.y());
    m.scale(scale.x(), scale.y());
    m.rotate(angle);
    m.pitch(pitch);
    m.translate(position.x(), position.y(), position.z());

    DrawKey key;
    key.shader = this->shader;
    key.smooth = this->smooth;
    // key.blend = this->blend;

    spine::RenderCommand *command = skeletonRenderer.render(skeleton);
    while (command) {
        assert(command->texture);
        float *positions = command->positions;
        float *uvs = command->uvs;
        uint32_t *colors = command->colors;
        uint16_t *indices = command->indices;
        key.image = static_cast<ImageRegion *>(command->texture)->img;
        float uvOffsetX = 0, uvOffsetY = 0;
        auto region = static_cast<ImageRegion *>(command->texture);
        if (region->uvx1() || region->uvy1() ||
            std::abs(region->uvx2() - 1) > 0.000001 ||
            std::abs(region->uvy2() - 1) > 0.000001) {
            uvOffsetX = region->uvx1();
            uvOffsetY = region->uvy1();
        }
        switch (command->blendMode) {
            case spine::BlendMode_Additive: {
                key.blend = Add;
                break;
            }
            case spine::BlendMode_Multiply: {
                key.blend = Multiply;
                break;
            }
            case spine::BlendMode_Screen: {
                key.blend = BlendScreen;
                break;
            }
            default: {
                key.blend = Alpha;
            }
        }
        Color c;
        for (int i = 0; i < command->numIndices;) {
            Triangle t, uv;
            uint16_t i1 = indices[i++];
            uint16_t i2 = indices[i++];
            uint16_t i3 = indices[i++];
            t.p1.setTo(positions[i1 * 2], positions[i1 * 2 + 1]);
            t.p2.setTo(positions[i2 * 2], positions[i2 * 2 + 1]);
            t.p3.setTo(positions[i3 * 2], positions[i3 * 2 + 1]);
            uv.p1.setTo(uvs[i1 * 2], uvs[i1 * 2 + 1]);
            uv.p2.setTo(uvs[i2 * 2], uvs[i2 * 2 + 1]);
            uv.p3.setTo(uvs[i3 * 2], uvs[i3 * 2 + 1]);
            if (uvOffsetX) {
                uv.p1.x() += uvOffsetX;
                uv.p2.x() += uvOffsetX;
                uv.p3.x() += uvOffsetX;
            }
            if (uvOffsetY) {
                uv.p1.y() += uvOffsetY;
                uv.p2.y() += uvOffsetY;
                uv.p3.y() += uvOffsetY;
            }
            c.setTo(colors[i1] & 0xffffff);
            c.a = ((colors[i1] >> 24) & 0xff) / 0xff;

            t = m * t;
            // t.debugPrint();
            ctx.addTriangle(key, t, uv, c.multiply(color), zIndex);
        }
        command = command->next;
    }
}

bool SpineSprite::hasAnimation(const std::string &name) {
    return !!this->skeletonData().findAnimation(spine::String(name.c_str()));
}

std::vector<std::string> SpineSprite::animationNames() {
    std::vector<std::string> result;
    auto all = this->skeletonData().getAnimations();
    for (size_t i = 0; i < all.size(); ++i) {
        result.emplace_back(all[i]->getName().buffer());
    }
    return result;
}

}
