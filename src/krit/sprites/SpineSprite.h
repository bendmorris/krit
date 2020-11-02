#ifndef KRIT_SPRITES_SPINESPRITE
#define KRIT_SPRITES_SPINESPRITE

#include "krit/asset/AssetCache.h"
#include "krit/asset/AssetContext.h"
#include "krit/asset/AssetLoader.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"
#include "krit/Sprite.h"
#include "spine/spine.h"
#include <memory>
#include <string>
#include <utility>

namespace krit {

struct SpineTextureLoader: public spine::TextureLoader {
    AssetCache *cache;

    SpineTextureLoader(AssetCache *cache): cache(cache) {}
    ~SpineTextureLoader() {}

    void load(spine::AtlasPage &page, const spine::String &path) override {
        std::shared_ptr<ImageData> texture = std::static_pointer_cast<ImageData>(this->cache->get(std::string(path.buffer())));
        ImageRegion *region = new ImageRegion(texture);
        page.setRendererObject(region);
        page.width = texture->width();
        page.height = texture->height();
    }

    void unload(void* texture) override {
        ImageRegion *region = static_cast<ImageRegion*>(texture);
        delete region;
    }
};

struct SpineLoader: public AssetLoader {
    AssetCache *cache;
    SpineTextureLoader textureLoader;

    SpineLoader(AssetCache *cache): cache(cache), textureLoader(cache) {}

    AssetType type() override { return SpineSkeletonAsset; }
    std::shared_ptr<void> loadAsset(const AssetInfo &info) override;
};

struct SkeletonBinaryData {
    spine::Atlas *atlas;
    spine::SkeletonBinary *binary;
    spine::SkeletonData *skeletonData;
    spine::AnimationStateData *animationStateData;

    SkeletonBinaryData(spine::Atlas* a, spine::SkeletonBinary* b, spine::SkeletonData* sd, spine::AnimationStateData* asd)
        : atlas(a), binary(b), skeletonData(sd), animationStateData(asd) {}
    ~SkeletonBinaryData() {
        delete this->animationStateData;
        delete this->skeletonData;
        delete this->binary;
        delete this->atlas;
    }
};

struct SpineSprite: public VisibleSprite {
    static float worldVertices[1024];
    static spine::String _customSkin;

    double angle = 0;
    double rate = 1;

    std::shared_ptr<SkeletonBinaryData> bin;
    spine::Skeleton *skeleton;
    spine::AnimationState *animationState;
    spine::Skin *skin;

    spine::SkeletonData &skeletonData() { return *this->bin->skeletonData; }
    spine::AnimationStateData &animationStateData() { return *this->bin->animationStateData; }

    SpineSprite(AssetContext &asset, const std::string &id);
    SpineSprite(AssetContext &asset, const AssetInfo &info);

    void setAnimation(size_t track, const std::string &name, bool loop = true, double speed = 1, float mix = -1);
    void addAnimation(size_t track, const std::string &name, bool loop = true, float delay = 0, float mix = -1);

    void setAttachment(const std::string &slot, const std::string &attachment) {
        this->skeleton->setAttachment(spine::String(slot.c_str()), spine::String(attachment.c_str()));
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
    void setDefaultMix(float t) {
        this->animationStateData().setDefaultMix(t);
    }

    void update(UpdateContext &ctx) override;
    virtual void render(RenderContext &ctx) override;
    Dimensions getSize() override { return Dimensions(0, 0); }
    void resize(double w, double h) override {}
    void advance(float time);
};

}

#endif
