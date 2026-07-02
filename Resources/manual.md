# DSK SFz Player — User Manual / Manual de Usuario

Version 1.0 · DSK Music · www.dskmusic.com

---

# ENGLISH

## Overview

DSK SFz Player is a 64-voice polyphonic SFZ/SF2 sampler available as a VST3 plugin and as a
standalone application. Load any SFZ or SF2 (SoundFont2) instrument and play it with full MIDI
control, five effects, and real-time parameter automation.

Supported audio formats inside SFZ libraries: WAV, FLAC, OGG. SF2 (SoundFont2) files are also
supported directly. Note: for SF2 instruments, the Amp ADSR and Filter/Mod (LFO) sections replace
the soundfont's own built-in envelope and filter with the plugin's knobs (same behaviour as SFZ).
The Amp ADSR is applied per note; Filter and Mod are applied to the mixed output rather than per
voice, since the SF2 engine renders all voices already mixed together.

---

## Loading an Instrument

### A  Drag & Drop (onto the plugin window)

| What you drop         | Result                                                          |
|-----------------------|-----------------------------------------------------------------|
| .sfz or .sf2 file     | Instrument loads immediately                                    |
| Folder with SFZ/SF2   | Folder is added to the Library panel                           |
| .zip pack             | You choose a destination, it extracts, then shows in Library   |

Only one SFZ/SF2 and one ZIP are loaded per drop; all dragged folders are added.

### B  Open Button

Click **Open SFZ/SF2/ZIP** (top-right of the main panel) to browse for .sfz, .sf2, or .zip files.

### C  Library Tree

Double-click any .sfz or .sf2 entry in the Library tree on the left.

If the SF2 file contains more than one bank/preset, a picker menu appears automatically after
loading — the same picker can be reopened at any time by clicking the instrument name shown in
the header (a small **▾** appears next to the name when a picker is available).

---

## Library Panel  (left column)

The Library organises your SFZ/SF2 collections in a folder tree.

### Adding a folder
Click **+** at the top of the panel and select a directory.
All .sfz, .sf2, and .zip files found inside (including subfolders) appear in the tree.

### Removing a folder
Select a root folder in the tree and click **−**.

### Expand / Collapse all
Click **+/−** to toggle all library root folders open or closed.

### Search
Type in the search box to filter instruments by name or path across all library folders.
Press **X** to clear the search and return to the tree view.

### Right-click on an SFZ or SF2 file
Opens a context menu with **Add to Favorites** (see Favorites section below).

---

## Favorites

The **★ FAVORITES** section always appears at the top of the Library tree.
It lets you pin the instruments you use most often.

### Adding an instrument to Favorites

Right-click any .sfz file in the library tree → **Add to Favorites**, then choose:

- **★ Favorites (no folder)** — pinned at the root level, no subfolder required
- An existing Favorites folder name — adds it there
- **New folder…** — creates a new named folder and adds the instrument to it

### Loading a favorite
Double-click any favorite entry.

### Right-click a favorite entry
- **Load** — loads the instrument
- **Move to…** — moves it to a different folder (or to root)
- **Remove from favorites** — removes it (asks for confirmation)

### Right-click a Favorites folder
- **Rename…** — rename the folder
- **Delete folder** — deletes the folder and all entries inside (asks for confirmation)

### Right-click the ★ FAVORITES header
- **New folder…** — create a new Favorites subfolder

---

## Parameters

### AMP Section

| Knob    | Range            | Description                              |
|---------|------------------|------------------------------------------|
| Volume  | −60 to +6 dB     | Master output level                      |
| Pan     | −100 to +100     | Stereo panning (L ← 0 → R)              |
| Semit.  | −24 to +24       | Pitch transpose in semitones             |
| Cents   | −100 to +100     | Fine pitch adjustment in cents           |
| A       | 0.001 – 10 s     | Amplitude Attack time                    |
| D       | 0.001 – 10 s     | Amplitude Decay time                     |
| S       | 0 – 100 %        | Amplitude Sustain level                  |
| R       | 0.001 – 10 s     | Amplitude Release time                   |

The **ADSR display** (to the right of the knobs) shows a live preview of the envelope shape.
The **L / R meters** show the current output level.

### Filter Section

| Knob   | Description                                             |
|--------|---------------------------------------------------------|
| Type   | LP (Low-Pass) · HP (High-Pass) · BP (Band-Pass) · Notch |
| Cutoff | Filter cutoff frequency — 20 Hz to 20 kHz              |
| Res    | Resonance / Q factor                                    |
| Env    | Filter envelope amount (−1 = full down, +1 = full up)  |
| A/D/S/R| Filter envelope shape (same ranges as AMP)             |

### Modulation (MOD) Section

Two independent LFOs (LFO 1 and LFO 2).

| Control | Options / Range          | Description              |
|---------|--------------------------|--------------------------|
| Shape   | Sine · Tri · Saw · Sq · S&H | LFO waveform shape    |
| Target  | Pitch · Cutoff · Pan · Vol  | Modulation destination |
| Rate    | 0.1 – 20 Hz              | LFO speed                |
| Depth   | 0 – 1                    | Modulation amount        |

### FX Section

Five independent effect slots, each enabled with its labelled toggle button.

| Effect | Parameters           | Description                          |
|--------|----------------------|--------------------------------------|
| DRIVE  | Drive · Mix          | Soft saturation / overdrive          |
| CHORUS | Rate · Depth · Mix   | Chorus modulation effect             |
| DELAY  | Time · Fbk · Mix     | Stereo echo (1 ms – 2000 ms)        |
| REVERB | Size · Damp · Mix    | Algorithmic reverb                   |
| EQ     | Low · Mid · High     | Three-band EQ (±12 dB per band)     |

### Footer

| Control  | Description                                      |
|----------|--------------------------------------------------|
| Voices   | Maximum simultaneous voices: 1 – 64             |
| RR Reset | Reset round-robin counters to the first sample  |

---

## Options Menu

Click **Options** (top-right) to access:

| Item                     | Description                                                  |
|--------------------------|--------------------------------------------------------------|
| All notes off            | Immediately silence all playing notes (MIDI panic)          |
| Reset round-robin …      | Restart round-robin sample cycling from the beginning        |
| Reset all parameters     | Restore every knob / button to its default value             |
| Export config…           | Save library folders + favorites + theme to a .json file     |
| Import config…           | Restore from a previously exported .json file                |
| Theme                    | Change the colour theme (see Themes section)                 |
| Help…                    | Open this manual                                             |
| About…                   | Plugin version and website link                              |

---

## Themes

Six colour themes are available under **Options → Theme**:

| Theme       | Style                          |
|-------------|--------------------------------|
| Deep Navy   | Dark — cyan accents (default)  |
| Midnight    | Dark — orange accents          |
| Forest      | Dark — green accents           |
| Deep Purple | Dark — purple accents          |
| Light Gray  | Light — blue accents           |
| Warm Cream  | Light — brown / red accents    |

The selected theme is saved automatically and restored on the next launch.
It is also included when you export the config file.

---

## Export & Import Config

### Export config…
Saves the following to a .json file of your choice:
- Library folder paths
- All Favorites (root entries and named folders)
- Current theme

### Import config…
Restores the above from a previously exported .json file.
The existing library and favorites are completely replaced by the imported data.

> **Note:** After extracting a .zip instrument pack, the Library shows only the
> newly extracted folder. Previous library entries are replaced.

---

## Tips & Shortcuts

- **PC Keyboard → MIDI:** Click the plugin window to focus it, then type to play notes
  (standard piano layout: A=C4, W=C#4, S=D4, E=D#4, D=E4, F=F4, etc.).
- **Hover tooltips:** Hover any knob or button to see its name and range.
- **Right-click knobs:** Shows a popup to type an exact value or reset to default.
- **Resizable window:** Drag the bottom-right corner to resize (min 860 × 360).
- **The banner (bottom of Library):** Clicking it opens www.dskmusic.com.

---
---

# ESPAÑOL

## Descripción general

DSK SFz Player es un sampler SFZ/SF2 polifónico de 64 voces disponible como plugin VST3 y como
aplicación independiente (Standalone). Carga cualquier instrumento en formato SFZ o SF2
(SoundFont2) y tócalo con control MIDI completo, cinco efectos y automatización de parámetros
en tiempo real.

Formatos de audio compatibles dentro de las librerías SFZ: WAV, FLAC, OGG. Los archivos SF2
(SoundFont2) también son compatibles directamente. Nota: en instrumentos SF2, las secciones Amp
ADSR y Filter/Mod (LFO) sustituyen la envolvente y el filtro propios del soundfont por los knobs
del plugin (mismo comportamiento que en SFZ). El ADSR de amplitud se aplica por nota; el Filtro y
el Mod se aplican sobre la mezcla final en vez de por voz, ya que el motor SF2 renderiza todas las
voces ya mezcladas.

---

## Cargar un instrumento

### A  Arrastrar y soltar (sobre la ventana del plugin)

| Qué arrastras            | Resultado                                                              |
|---------------------------|------------------------------------------------------------------------|
| Archivo .sfz o .sf2       | El instrumento se carga inmediatamente                                 |
| Carpeta con SFZ/SF2       | La carpeta se añade al panel de Librería                              |
| Pack .zip                 | Eliges dónde extraer, se descomprime y aparece en la Librería         |

Solo se carga un SFZ/SF2 y un ZIP por soltar; todas las carpetas arrastradas se añaden.

### B  Botón Open

Haz clic en **Open SFZ/SF2/ZIP** (arriba a la derecha) para buscar archivos .sfz, .sf2 o .zip.

### C  Árbol de Librería

Haz doble clic en cualquier entrada .sfz o .sf2 del árbol de la Librería (columna izquierda).

Si el archivo SF2 contiene más de un banco/preset, aparece automáticamente un menú selector
tras la carga — el mismo selector puede volver a abrirse en cualquier momento haciendo clic en
el nombre del instrumento mostrado en el encabezado (aparece un pequeño **▾** junto al nombre
cuando el selector está disponible).

---

## Panel de Librería  (columna izquierda)

La Librería organiza tus colecciones SFZ/SF2 en un árbol de carpetas.

### Añadir una carpeta
Haz clic en **+** en la parte superior del panel y selecciona un directorio.
Todos los archivos .sfz, .sf2 y .zip que contenga (incluyendo subcarpetas) aparecerán en el árbol.

### Eliminar una carpeta
Selecciona una carpeta raíz en el árbol y haz clic en **−**.

### Expandir / Contraer todo
Haz clic en **+/−** para abrir o cerrar todas las carpetas raíz de la librería.

### Búsqueda
Escribe en el cuadro de búsqueda para filtrar instrumentos por nombre o ruta.
Pulsa **X** para limpiar la búsqueda y volver a la vista de árbol.

### Clic derecho en un archivo SFZ o SF2
Abre un menú contextual con **Add to Favorites** (ver sección de Favoritos).

---

## Favoritos

La sección **★ FAVORITES** aparece siempre en la parte superior del árbol de la Librería.
Permite fijar los instrumentos que más uses.

### Añadir un instrumento a Favoritos

Haz clic derecho en cualquier archivo .sfz → **Add to Favorites**, luego elige:

- **★ Favorites (no folder)** — se fija en el nivel raíz, sin necesidad de subcarpeta
- El nombre de una carpeta de Favoritos existente — se añade allí
- **New folder…** — crea una nueva carpeta con nombre y añade el instrumento

### Cargar un favorito
Haz doble clic en cualquier entrada de favoritos.

### Clic derecho en una entrada de favoritos
- **Load** — carga el instrumento
- **Move to…** — muévelo a otra carpeta (o a la raíz)
- **Remove from favorites** — lo elimina (pide confirmación)

### Clic derecho en una carpeta de Favoritos
- **Rename…** — cambia el nombre de la carpeta
- **Delete folder** — elimina la carpeta y todas sus entradas (pide confirmación)

### Clic derecho en la cabecera ★ FAVORITES
- **New folder…** — crea una nueva subcarpeta de Favoritos

---

## Parámetros

### Sección AMP

| Knob    | Rango            | Descripción                              |
|---------|------------------|------------------------------------------|
| Volume  | −60 a +6 dB      | Nivel de salida maestro                  |
| Pan     | −100 a +100      | Panorama estéreo (Izq ← 0 → Der)        |
| Semit.  | −24 a +24        | Transposición en semitonos               |
| Cents   | −100 a +100      | Afinación fina en centésimas             |
| A       | 0.001 – 10 s     | Tiempo de Ataque de la amplitud          |
| D       | 0.001 – 10 s     | Tiempo de Caída (Decay)                  |
| S       | 0 – 100 %        | Nivel de Sostenimiento (Sustain)         |
| R       | 0.001 – 10 s     | Tiempo de Liberación (Release)           |

La **visualización ADSR** (a la derecha de los knobs) muestra una previsualización en tiempo real.
Los **medidores L / R** muestran el nivel de salida actual.

### Sección Filter (Filtro)

| Knob   | Descripción                                                      |
|--------|------------------------------------------------------------------|
| Type   | LP (Paso Bajo) · HP (Paso Alto) · BP (Paso de Banda) · Notch   |
| Cutoff | Frecuencia de corte — 20 Hz a 20 kHz                            |
| Res    | Resonancia / Factor Q                                            |
| Env    | Cantidad de envolvente (−1 = baja, +1 = sube)                   |
| A/D/S/R| Forma de la envolvente del filtro (mismos rangos que AMP)       |

### Sección de Modulación (MOD)

Dos LFOs independientes (LFO 1 y LFO 2).

| Control | Opciones / Rango               | Descripción              |
|---------|--------------------------------|--------------------------|
| Shape   | Sine · Tri · Saw · Sq · S&H   | Forma de onda del LFO    |
| Target  | Pitch · Cutoff · Pan · Vol     | Destino de modulación    |
| Rate    | 0.1 – 20 Hz                    | Velocidad del LFO        |
| Depth   | 0 – 1                          | Cantidad de modulación   |

### Sección FX (Efectos)

Cinco ranuras de efectos independientes, cada una activada con su botón.

| Efecto | Parámetros           | Descripción                             |
|--------|----------------------|-----------------------------------------|
| DRIVE  | Drive · Mix          | Saturación suave / overdrive            |
| CHORUS | Rate · Depth · Mix   | Efecto de chorus con modulación         |
| DELAY  | Time · Fbk · Mix     | Eco estéreo (1 ms – 2000 ms)           |
| REVERB | Size · Damp · Mix    | Reverberación algorítmica               |
| EQ     | Low · Mid · High     | Ecualizador de tres bandas (±12 dB)    |

### Pie de página

| Control  | Descripción                                                  |
|----------|--------------------------------------------------------------|
| Voices   | Número máximo de voces simultáneas: 1 – 64                  |
| RR Reset | Reinicia los contadores de round-robin a la primera muestra  |

---

## Menú Options

Haz clic en **Options** (arriba a la derecha) para acceder a:

| Opción                   | Descripción                                                          |
|--------------------------|----------------------------------------------------------------------|
| All notes off            | Silencia inmediatamente todas las notas (pánico MIDI)               |
| Reset round-robin …      | Reinicia el ciclo de muestras round-robin desde el principio        |
| Reset all parameters     | Restaura todos los controles a sus valores por defecto              |
| Export config…           | Guarda librería + favoritos + tema en un archivo .json              |
| Import config…           | Restaura desde un archivo .json exportado anteriormente             |
| Theme                    | Cambia el tema de color (ver sección Temas)                         |
| Help…                    | Abre este manual                                                     |
| About…                   | Versión del plugin y enlace a la web                                |

---

## Temas (Themes)

Hay seis temas de color disponibles en **Options → Theme**:

| Tema        | Estilo                               |
|-------------|--------------------------------------|
| Deep Navy   | Oscuro — acentos cyan (predeterminado) |
| Midnight    | Oscuro — acentos naranja             |
| Forest      | Oscuro — acentos verde               |
| Deep Purple | Oscuro — acentos púrpura             |
| Light Gray  | Claro — acentos azul                 |
| Warm Cream  | Claro — acentos marrón / rojo        |

El tema seleccionado se guarda automáticamente y se restaura al siguiente inicio.
También se incluye al exportar el archivo de configuración.

---

## Exportar e Importar configuración

### Export config…
Guarda lo siguiente en un archivo .json:
- Rutas de las carpetas de la librería
- Todos los Favoritos (entradas raíz y carpetas con nombre)
- Tema actual

### Import config…
Restaura lo anterior desde un archivo .json exportado previamente.
La librería y los favoritos existentes se reemplazan completamente con los datos importados.

> **Nota:** Al extraer un pack .zip de instrumentos, la Librería mostrará únicamente
> la carpeta recién extraída. Las entradas anteriores de la librería se reemplazan.

---

## Consejos y atajos

- **Teclado del PC → MIDI:** Haz clic en la ventana del plugin para darle el foco y
  escribe para tocar notas (A=Do4, W=Do#4, S=Re4, E=Re#4, D=Mi4, F=Fa4, etc.).
- **Tooltips al pasar el cursor:** Pasa el cursor sobre cualquier knob o botón para ver
  su nombre y rango.
- **Clic derecho en los knobs:** Muestra un menú para introducir un valor exacto o restablecer.
- **Ventana redimensionable:** Arrastra la esquina inferior derecha (mín. 860 × 360).
- **El banner (parte inferior de la Librería):** Al hacer clic abre www.dskmusic.com.

---

*DSK Music · www.dskmusic.com*
