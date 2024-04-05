#ifndef __TEXTURE__
#define __TEXTURE__

#include "core.h"

GLIB_NAMESPACE_BEGIN

struct TextureConfig
{
    TextureType type{TEXTURE_2D};
    unsigned int samples{1};
    unsigned int layers{1};

    unsigned int format{GL_RGBA};
    int internalFormat{GL_RGBA8};
    unsigned int dataType{GL_UNSIGNED_BYTE};

    int level{0};
    int border{0};

    bool anisotropicFilter{true};
    float anisotopicFilterValue{16.0f};

    bool useMipmaps{true};
    int magFilter{GL_LINEAR};
    int minFilter{GL_LINEAR_MIPMAP_LINEAR};

    int wrapT{GL_REPEAT};
    int wrapS{GL_REPEAT};
    int wrapR{GL_REPEAT};
};
struct Image
{
    std::string path{""};
    unsigned char *data{nullptr};
};

class Texture
{
protected:
    unsigned int m_id;

    Extent2D m_extent{};

    TextureConfig m_config{};

    Image m_image{};

    bool m_generated{false};

    void setup() const;

public:
    Texture(Extent2D extent) : m_extent(extent) {}
    Texture(Extent2D extent, TextureConfig config) : m_extent(extent), m_config(config) {}
    Texture(std::string imagePath) : m_image({imagePath, nullptr}) {}
    Texture(std::string imagePath, TextureConfig config) : m_image({imagePath, nullptr}), m_config(config) {}
    ~Texture()
    {
        GL_CHECK(glDeleteTextures(1, &m_id));
    }

    void generate();

    inline unsigned int get_id() const { return m_id; }

    inline Image get_image() const { return m_image; }
    inline void set_image(Image img) { m_image = img; }

    inline Extent2D get_extent() const { return m_extent; }
    void set_extent(Extent2D extent);

    inline TextureConfig get_config() const { return m_config; }

    inline void set_config(TextureConfig config) { m_config = config; }

    virtual void bind(unsigned int slot = 0) const;

    virtual void unbind() const;

    virtual void resize(Extent2D extent);

    inline bool is_generated() const { return m_generated; }
};

GLIB_NAMESPACE_END

#endif