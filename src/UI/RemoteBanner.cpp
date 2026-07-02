#include "RemoteBanner.h"

namespace RemoteBanner
{
    static const char* kBannerConfigUrl = "https://www.dskmusic.com/downloads/ads/dsksfzplayer/banner.json";
    static const char* kTrackUrl        = "https://www.dskmusic.com/downloads/ads/dsksfzplayer/track.php";

    Result fetchBlocking()
    {
        Result result;

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withConnectionTimeoutMs(4000);

        std::unique_ptr<juce::InputStream> jsonStream(juce::URL(kBannerConfigUrl).createInputStream(options));
        if (jsonStream == nullptr)
            return result;

        const juce::String jsonText = jsonStream->readEntireStreamAsString();
        auto parsed = juce::JSON::parse(jsonText);
        if (!parsed.isObject())
            return result;

        const juce::String imageUrl = parsed.getProperty("image", juce::String()).toString().trim();
        const juce::String destUrl  = parsed.getProperty("url",   juce::String()).toString().trim();
        if (imageUrl.isEmpty() || destUrl.isEmpty())
            return result;

        std::unique_ptr<juce::InputStream> imgStream(juce::URL(imageUrl).createInputStream(options));
        if (imgStream == nullptr)
            return result;

        juce::MemoryBlock imgData;
        imgStream->readIntoMemoryBlock(imgData);
        if (imgData.getSize() == 0)
            return result;

        juce::Image img = juce::ImageFileFormat::loadFrom(imgData.getData(), imgData.getSize());
        if (!img.isValid())
            return result;

        result.ok    = true;
        result.image = img;
        result.url   = destUrl;
        return result;
    }

    void trackClickAsync(const juce::String& destinationUrl)
    {
        juce::Thread::launch([destinationUrl]
        {
            auto url = juce::URL(kTrackUrl)
                           .withParameter("url",  destinationUrl)
                           .withParameter("os",   juce::SystemStats::getOperatingSystemName())
                           .withParameter("ver",  ProjectInfo::versionString)
                           .withParameter("lang", juce::SystemStats::getUserLanguage());

            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                               .withConnectionTimeoutMs(3000);

            // Solo se necesita completar la petición GET; no importa la respuesta.
            std::unique_ptr<juce::InputStream> stream(url.createInputStream(options));
            if (stream != nullptr)
                stream->readEntireStreamAsString();
        });
    }
}
