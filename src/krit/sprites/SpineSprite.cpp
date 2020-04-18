#include "krit/asset/ImageLoader.h"
#include "krit/sprites/SpineSprite.h"
#include "krit/render/ImageRegion.h"
#include <spine/Extension.h>

spine::SpineExtension *spine::getDefaultExtension() {
   return new spine::DefaultSpineExtension();
}

using namespace std;
using namespace krit;

namespace krit {

string SpineLoader::TYPE = "sp";
float SpineSprite::worldVertices[1024] = {0};

shared_ptr<void> SpineLoader::loadAsset(string id) {
    const spine::String &atlasName((id + ".atlas").c_str());
    const spine::String &skelName((id + ".skel").c_str());
    spine::Atlas *atlas = new spine::Atlas(atlasName, &this->textureLoader);
    spine::SkeletonBinary *binary = new spine::SkeletonBinary(atlas);
    spine::SkeletonData *skeletonData = binary->readSkeletonDataFile(skelName);
    spine::AnimationStateData *animationStateData = new spine::AnimationStateData(skeletonData);
    shared_ptr<SkeletonBinaryData> bin = make_shared<SkeletonBinaryData>(
        atlas,
        binary,
        skeletonData,
        animationStateData
    );
    return static_pointer_cast<void>(bin);
}

void SpineSprite::setAnimation(size_t track, const string &name, bool loop, double speed, float mix) {
    auto trackEntry = this->animationState->setAnimation(track, spine::String(name.c_str()), loop);
    if (speed != 1) {
        trackEntry->setTimeScale(speed);
    }
    if (mix >= 0) {
        trackEntry->setMixTime(mix);
    }
}

void SpineSprite::addAnimation(size_t track, const string &name, bool loop, float delay, float mix) {
    auto trackEntry = this->animationState->addAnimation(track, spine::String(name.c_str()), loop, delay);
    if (mix >= 0) {
        trackEntry->setMixTime(mix);
    }
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

void SpineSprite::setSkin(const string &name) {
    spine::Skin *newSkin = this->skeletonData().findSkin(spine::String(name.c_str()));
    if (newSkin) {
        this->skin->addSkin(newSkin);
    } else {
        panic("unknown skin: %s\n", name.c_str());
    }
    this->skeleton->setSkin(this->skin);
    this->skeleton->setSlotsToSetupPose();
    this->animationState->apply(*this->skeleton);
}

void SpineSprite::removeSkin(const string &name) {
    spine::Skin *oldSkin = this->skeletonData().findSkin(spine::String(name.c_str()));
    spine::Skin::AttachmentMap::Entries entries = oldSkin->getAttachments();
    while (entries.hasNext()) {
        spine::Skin::AttachmentMap::Entry &entry = entries.next();
        skin->removeAttachment(entry._slotIndex, entry._name);
    }
}

void SpineSprite::update(UpdateContext &ctx) {
    animationState->setTimeScale(this->rate);
    double elapsed = ctx.elapsed;
    if (elapsed > 0) {
        this->advance(elapsed);
    }
}

static void _applyArc(FloatPoint &base, FloatPoint &tip, float arc) {
    FloatPoint diff(tip);
    diff.subtract(base).multiply(pow(1 - arc, 2));
    base.add(diff);
}

void SpineSprite::render(RenderContext &ctx) {
    switch (this->effect) {
        case GhostEffect: {
            this->advance(-this->ghost.time * this->ghost.frames);
            for (int i = 0; i < this->ghost.frames; ++i) {
                Color c(this->ghost.color);
                c.a = (this->color.a * (i + 1) / this->ghost.frames) * this->ghost.color.a;
                this->_render(ctx, true, this->ghost.slot, c);
                this->advance(this->ghost.time);
            }
            break;
        }
        case TrailEffect: {
            auto slotTip = this->skeleton->getSlots()[this->trail.tip],
                slotBase = this->skeleton->getSlots()[this->trail.base];
            if (!(slotTip || slotBase)) {
                break;
            }
            double trailTime = this->trail.time;
            this->trail.elapsed += ctx.elapsed;
            if (trailTime > 0) {
                auto slotTrack = this->trail.trackPart > -1 ? this->skeleton->getSlots()[this->trail.trackPart] : nullptr;
                float inc = -trailTime / this->trail.slices;
                DrawKey key;
                key.smooth = SmoothLinear;
                key.blend = BlendMode::Add;
                FloatPoint lastTip, lastBase, curTip, curBase;
                float total = 0;
                int start = this->trail.slices * (1 - min(1.0, this->trail.elapsed / this->trail.time));
                for (int i = start; i < this->trail.slices; ++i) {
                    auto attachTip = slotTip->getAttachment(),
                        attachBase = slotBase->getAttachment();
                    if (total - inc > this->trail.elapsed) {
                        break;
                    } else {
                        total -= inc;
                    }
                    if (!(attachTip || attachBase) ||
                        !attachTip->getRTTI().isExactly(spine::PointAttachment::rtti) ||
                        !attachBase->getRTTI().isExactly(spine::PointAttachment::rtti))
                    {
                        this->advance(inc);
                        continue;
                    }
                    spine::PointAttachment
                        *pointTip = static_cast<spine::PointAttachment*>(attachTip),
                        *pointBase = static_cast<spine::PointAttachment*>(attachBase);
                    pointTip->computeWorldPosition(slotTip->getBone(), curTip.x, curTip.y);
                    pointBase->computeWorldPosition(slotBase->getBone(), curBase.x, curBase.y);
                    if (i > this->trail.slices - this->trail.arc) {
                        _applyArc(curBase, curTip, static_cast<float>(this->trail.slices - i) / this->trail.arc);
                    }
                    if ((!slotTrack || slotTrack->getAttachment()) && lastTip.x && lastTip.y && curTip.x && curTip.y && lastTip.x != curTip.x && lastTip.y != curTip.y) {
                        Color c(this->trail.color);
                        c.a = this->trail.color.a * (1 - static_cast<float>(i) / this->trail.slices);
                        _renderTrail(key, ctx, lastTip, lastBase, curTip, curBase, c);
                    }
                    this->advance(inc);
                    lastTip.setTo(curTip);
                    lastBase.setTo(curBase);
                }
                this->advance(total);
            }
            break;
        }
    }
    this->_render(ctx, false, 0, Color::black());
}

void SpineSprite::_renderTrail(DrawKey &key, RenderContext &ctx, FloatPoint lastTip, FloatPoint lastBase, FloatPoint curTip, FloatPoint curBase, Color &color) {
    Triangle t1(lastTip.x, lastTip.y, lastBase.x, lastBase.y, curTip.x, curTip.y);
    Triangle t2(lastBase.x, lastBase.y, curTip.x, curTip.y, curBase.x, curBase.y);
    t1.scale(this->scale.x, this->scale.y);
    t1.translate(this->position);
    t2.scale(this->scale.x, this->scale.y);
    t2.translate(this->position);
    ctx.addTriangle(key, t1, t1, color);
    ctx.addTriangle(key, t2, t2, color);
}

void SpineSprite::_render(RenderContext &ctx, bool ghost, int ghostSlot, Color ghostColor) {
    spine::Skeleton &skeleton = *this->skeleton;
    spine::Vector<spine::Slot*> &drawOrder = skeleton.getDrawOrder();
    size_t size = drawOrder.size();
    for (int i = 0; i < size; ++i) {
        spine::Slot *slot = drawOrder[i];
        if (ghost && ((ghostSlot != -1) && slot->getData().getIndex() != ghostSlot)) {
            continue;
        }
        Color color;
        if (ghost) {
            color = ghostColor;
            color.a = ghostColor.a * skeleton.getColor().a * slot->getColor().a;
        } else {
            color.r = this->color.r * skeleton.getColor().r * slot->getColor().r;
            color.g = this->color.g * skeleton.getColor().g * slot->getColor().g;
            color.b = this->color.b * skeleton.getColor().b * slot->getColor().b;
            color.a = this->color.a * skeleton.getColor().a * slot->getColor().a;
        }
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
            spine::RegionAttachment *regionAttachment = static_cast<spine::RegionAttachment*>(attachment);
            ImageRegion *region = static_cast<ImageRegion*>((static_cast<spine::AtlasRegion*>(regionAttachment->getRendererObject()))->page->getRendererObject());
            regionAttachment->computeWorldVertices(slot->getBone(), worldVertices, 0, 2);
            DrawKey key;
            key.image = region->img;
            key.smooth = this->smooth;
            key.blend = blendMode;
            spine::Vector<float> &uvs = regionAttachment->getUVs();
            Triangle t1(worldVertices[0], worldVertices[1], worldVertices[2], worldVertices[3], worldVertices[4], worldVertices[5]);
            Triangle t2(worldVertices[4], worldVertices[5], worldVertices[6], worldVertices[7], worldVertices[0], worldVertices[1]);
            t1.scale(this->scale.x, this->scale.y);
            t1.translate(this->position);
            t2.scale(this->scale.x, this->scale.y);
            t2.translate(this->position);
            Triangle uvt1(uvs[0], uvs[1], uvs[2], uvs[3], uvs[4], uvs[5]);
            Triangle uvt2(uvs[4], uvs[5], uvs[6], uvs[7], uvs[0], uvs[1]);
            ctx.addTriangle(
                key,
                t1,
                uvt1,
                color
            );
            ctx.addTriangle(
                key,
                t2,
                uvt2,
                color
            );
        } else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            // spine::MeshAttachment *mesh = static_cast<spine::MeshAttachment*>(attachment);
            // SpineSprite.worldVertices.ensureSize(mesh.trianglesCount * 6);
            // var worldVertices = SpineSprite.worldVertices.ref();
            // var image = (mesh.rendererObject as Ptr[spAtlasRegion]).page.rendererObject as Ptr[ImageData];
            // var imageRegion = ImageRegion.fromImage(image);
            // // before rendering via spSkeleton_updateWorldTransform
            // spVertexAttachment_computeWorldVertices(mesh.super, slot, 0, mesh.super.worldVerticesLength, worldVertices, 0, 2);
            // var key = struct DrawKey {
            //     image: imageRegion.image,
            //     smooth: this->smooth,
            //     blend: blendMode,
            // };
            // var uvs: Ptr[Float] = mesh.uvs;
            // for i in 0 ... mesh.trianglesCount / 3 {
            //     var i0 = mesh.triangles[i * 3] << 1;
            //     var i1 = mesh.triangles[i * 3 + 1] << 1;
            //     var i2 = mesh.triangles[i * 3 + 2] << 1;
            //     var t = T(worldVertices[i0], worldVertices[i0 + 1], worldVertices[i1], worldVertices[i1 + 1], worldVertices[i2], worldVertices[i2 + 1]);
            //     t.multiply(this->scale.fullScaleX, this->scale.fullScaleY);
            //     context.addTriangle(
            //         key,
            //         t,
            //         T(uvs[i0], uvs[i0 + 1], uvs[i1], uvs[i1 + 1], uvs[i2], uvs[i2 + 1]),
            //         color, false
            //     );
            // }
        }
    }
}

}
