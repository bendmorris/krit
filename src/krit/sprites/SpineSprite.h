#ifndef KRIT_SPRITES_SPINESPRITE
#define KRIT_SPRITES_SPINESPRITE

#include <memory>
#include <stddef.h>
#include <string>
#include <utility>

#include "krit/App.h"
#include "krit/Engine.h"
#include "krit/Sprite.h"
#include "krit/asset/AssetCache.h"
#include "krit/asset/AssetLoader.h"
#include "krit/math/Dimensions.h"
#include "krit/render/ImageData.h"
#include "krit/render/ImageRegion.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"
#include "spine/AnimationStateData.h"
#include "spine/Atlas.h"
#include "spine/Skeleton.h"
#include "spine/SkeletonBinary.h"
// #include "spine/SkeletonJson.h"
#include "spine/SkeletonData.h"
#include "spine/Skin.h"
#include "spine/SpineString.h"
#include "spine/TextureLoader.h"
#include "spine/spine.h"

namespace spine {

class AnimationState;

}

namespace krit {

struct UpdateContext;

struct SpineTextureLoader : public spine::TextureLoader {
    static SpineTextureLoader instance;

    void load(spine::AtlasPage &page, const spine::String &path) override;
    void unload(void *texture) override;
};

struct SpineData {
    std::unique_ptr<spine::Atlas> atlas;
    std::unique_ptr<spine::SkeletonBinary> binary;
    // std::unique_ptr<spine::SkeletonJson> json;
    std::unique_ptr<spine::SkeletonData> skeletonData;
    std::unique_ptr<spine::AnimationStateData> animationStateData;
};

struct SpineSprite : public VisibleSprite {
    static float worldVertices[1024];
    static std::string defaultAtlasPath;
    static spine::String _customSkin;

    static void setAtlasPath(const std::string &s) { defaultAtlasPath = s; }

    float angle = 0;
    float rate = 1;

    std::shared_ptr<SpineData> data;
    std::unique_ptr<spine::Skeleton> skeleton;
    std::unique_ptr<spine::AnimationState> animationState;
    std::unique_ptr<spine::Skin> skin;

    spine::SkeletonData &skeletonData() { return *data->skeletonData; }
    spine::AnimationStateData &animationStateData() {
        return *data->animationStateData;
    }

    SpineSprite(const std::string &id);
    virtual ~SpineSprite() {}

    float setAnimation(size_t track, const std::string &name, bool loop = true,
                       float speed = 1, float mix = -1);
    float addAnimation(size_t track, const std::string &name, bool loop = true,
                       float delay = 0, float mix = -1);
    const char *getAnimation(size_t track);

    void setAttachment(const std::string &slot, const std::string &attachment) {
        this->skeleton->setAttachment(spine::String(slot.c_str()),
                                      spine::String(attachment.c_str()));
    }

    void reset() {
        this->skeleton->setToSetupPose();
        if (this->skin) {
            auto entries = this->skin->getAttachments();
            while (entries.hasNext()) {
                auto entry = entries.next();
                this->skin->removeAttachment(entry._slotIndex, entry._name);
            }
        }
    }

    void setSkin(const std::string &name);
    void removeSkin(const std::string &name);

    /**
     * This will be shared across all instances of this skeleton as long as the
     * skeleton is live in the asset cache.
     */
    void setDefaultMix(float t) { this->animationStateData().setDefaultMix(t); }

    void update(UpdateContext &ctx) override;
    virtual void render(RenderContext &ctx) override;
    Dimensions getSize() override { return Dimensions(0, 0); }
    void resize(float w, float h) override {}
    void advance(float time);
};

}

#endif
