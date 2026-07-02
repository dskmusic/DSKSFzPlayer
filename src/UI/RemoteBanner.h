#pragma once
#include <JuceHeader.h>

// ══════════════════════════════════════════════════════════════════════════════
// RemoteBanner — descarga en segundo plano un banner.json remoto (imagen + URL
// destino) para reemplazar el banner embebido en Resources/banner.png. Si la red
// falla, el JSON no es válido, o la imagen no se puede decodificar, el llamador
// simplemente conserva el banner/URL embebidos (no hay ningún estado de error
// que gestionar aquí: fetchBlocking() devuelve Result::ok = false).
// ══════════════════════════════════════════════════════════════════════════════
namespace RemoteBanner
{
    struct Result
    {
        bool ok = false;
        juce::Image image;
        juce::String url;
    };

    // Llamada BLOQUEANTE (red) — solo debe invocarse desde un hilo en segundo
    // plano, nunca desde el hilo de mensajes ni desde el hilo de audio.
    Result fetchBlocking();

    // Envía de forma "fire-and-forget" (no bloqueante) un aviso de click al
    // servidor para estadísticas. Lanza su propio hilo interno y no requiere
    // ningún callback ni gestión de resultado por parte del llamador.
    void trackClickAsync(const juce::String& destinationUrl);
}
