#include "krit/sprites/SpineSprite.h"

#include <spine/Extension.h>

#include "krit/App.h"
#include "krit/UpdateContext.h"
#include "krit/asset/AssetInfo.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/math/ScaleFactor.h"
#include "krit/math/Triangle.h"
#include "krit/render/BlendMode.h"
#include "krit/render/DrawKey.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"
#include "spine/AnimationState.h"
#include "spine/Attachment.h"
#include "spine/Bone.h"
#include "spine/Color.h"
#include "spine/MeshAttachment.h"
#include "spine/RTTI.h"
#include "spine/RegionAttachment.h"
#include "spine/Slot.h"
#include "spine/Vector.h"

struct KritSpineExtension : public spine::DefaultSpineExtension {
    char *_readFile(const spine::String &path, int *length) override {
        return krit::IoRead::read(std::string(path.buffer()), length);
    }
};

spine::SpineExtension *spine::getDefaultExtension() {
    return new KritSpineExtension();
}

namespace krit {

SpineTextureLoader SpineTextureLoader::instance;

SpineSprite::SpineSprite(const std::string &id)
    : SpineSprite(Assets::byPath(id)) {}

SpineSprite::SpineSprite(const AssetInfo &info) {
    spine::Bone::setYDown(true);

    // load skeleton/animation data
    this->bin = App::ctx.engine->getSpine(info);
    this->skeleton = new spine::Skeleton(&this->skeletonData());
    this->animationState =
        new spine::AnimationState(&this->animationStateData());
    this->skin = new spine::Skin(spine::String("custom"));
}

SpineSprite::~SpineSprite() {
    delete this->skeleton;
    delete this->animationState;
    delete this->skin;
}

float SpineSprite::worldVertices[1024] = {0};

template <>
SkeletonBinaryData *
AssetLoader<SkeletonBinaryData>::loadAsset(const AssetInfo &info) {
    const spine::String &atlasName(
        (info.path.substr(0, info.path.length() - 5) + ".atlas").c_str());
    const spine::String &skelName((info.path).c_str());
    spine::Atlas *atlas =
        new spine::Atlas(atlasName, &SpineTextureLoader::instance);
    spine::SkeletonBinary *binary = new spine::SkeletonBinary(atlas);
    spine::SkeletonData *skeletonData = binary->readSkeletonDataFile(skelName);
    spine::AnimationStateData *animationStateData =
        new spine::AnimationStateData(skeletonData);
    SkeletonBinaryData *bin =
        new SkeletonBinaryData(atlas, binary, skeletonData, animationStateData);
    return bin;
}

template <>
void AssetLoader<SkeletonBinaryData>::unloadAsset(SkeletonBinaryData *bin) {
    delete bin;
}

float SpineSprite::setAnimation(size_t track, const std::string &name,
                                bool loop, float speed, float mix) {
    auto trackEntry = this->animationState->setAnimation(
        track, spine::String(name.c_str()), loop);
    if (speed != 1) {
        trackEntry->setTimeScale(speed);
    }
    if (mix >= 0) {
        trackEntry->setMixTime(mix);
    }
    return trackEntry->getAnimationEnd();
}

float SpineSprite::addAnimation(size_t track, const std::string &name,
                                bool loop, float delay, float mix) {
    auto trackEntry = this->animationState->addAnimation(
        track, spine::String(name.c_str()), loop, delay);
    if (mix >= 0) {
        trackEntry->setMixTime(mix);
    }
    return trackEntry->getAnimationEnd();
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
    this->skeleton->setSkin(this->skin);
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
            ImageRegion *region = static_cast<ImageRegion *>(
                (static_cast<spine::AtlasRegion *>(
                     regionAttachment->getRendererObject()))
                    ->page->getRendererObject());
            regionAttachment->computeWorldVertices(slot->getBone(),
                                                   worldVertices, 0, 2);
            DrawKey key;
            key.image = region->img;
            key.smooth = this->smooth;
            key.blend = blendMode;
            spine::Vector<float> &uvs = regionAttachment->getUVs();
            Triangle t1(worldVertices[0], worldVertices[1], worldVertices[2],
                        worldVertices[3], worldVertices[4], worldVertices[5]);
            Triangle t2(worldVertices[4], worldVertices[5], worldVertices[6],
                        worldVertices[7], worldVertices[0], worldVertices[1]);
            t1.scale(this->scale.x, this->scale.y);
            t1.translate(this->position);
            t2.scale(this->scale.x, this->scale.y);
            t2.translate(this->position);
            Triangle uvt1(uvs[0], uvs[1], uvs[2], uvs[3], uvs[4], uvs[5]);
            Triangle uvt2(uvs[4], uvs[5], uvs[6], uvs[7], uvs[0], uvs[1]);
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
                t.scale(this->scale.x, this->scale.y);
                t.translate(this->position);
                Triangle uvt(uvs[i0], uvs[i0 + 1], uvs[i1], uvs[i1 + 1],
                             uvs[i2], uvs[i2 + 1]);
                ctx.addTriangle(key, t, uvt, color);
            }
        }
    }
}

}
