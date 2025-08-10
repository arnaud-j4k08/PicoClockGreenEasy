#include "UiTexts.h"
#include "Settings.h"
#include "PicoClockHw/Wifi.h"

template <>
const char *g_textTable<TextId>[][LanguageCount] =
{
    // Main menu
    {"Alarms", "Alarmes"},
    {"Tools", "Outils"},
    {"Clock sync", "Synchro"},
    {"Options", "Options"},
    {"Exit", "Sortir"},

    // Alarms
    {"Next: ", "Prochain: "},
    {"Al 1: ", "Al 1: "},
    {"Al 2: ", "Al 2: "},
    {"Volume: ", "Volume: "},
    {"Track: ", "Piste: "},
    {"Random", "Aleatoire"},
    {"Skip next alarm: ", "Ignorer prochain: "},
    {"Once", "Une fois"},
    {"Weekly", "Hebdo"},

    // Options
    {"Auto light: ", "Luminosite auto: "},
    {"Time format: ", "Heure: Format: "},
    {"24h", "24h"},
    {"12h", "12h"},
    {"Date format: ", "Date: Format: "},
    {"Hourly chime: ", "Carillon horaire: "},
    {"Auto scroll: ", "Defilement auto: "},
    {"Digit animation: ", "Animation des chiffres: "},
    {"Brightness: ", "Reglage Luminosite: "},
    {"Dark brightness: ", "Env noir: Luminosite: "},
    {"Dim brightness: ", "Env sombre: Luminosite: "},
    {"Max brightness: ", "Env clair: Luminosite: "},
    {"On", "Oui"},
    {"Off", "Non"},
    {"Chime volume: ", "Volume carillon: "},

    // Tools menu, stopwatch and countdown
    {"Flashlight", "Lampe"},
    {"Stopwatch", "Chrono"},
    {"Reset", "Remettre a zero"},
    {"Countdown", "Minuterie"},
    {"Set", "Regler"},

    // Sync menu
    {"Source: ", "Source: "},
    {"Sync now", "Synchroniser"},
    {"Daily sync time: ", "Heure synchro quotidienne: "},
    {"Last sync: ", "Derniere synchro: "},
    {"Last drift: ", "Decalage derniere synchro: "},
    {"Wifi: ", "Wifi: "},
};

template <>
const char *g_textTable<Wifi::Status>[][LanguageCount] =
{
    {"Unknown", "Inconnu"},
    {"OK", "OK"},
    {"Not available", "Non disponible"},
    {"Down", "Hors service"},
    {"Connecting", "Connexion"},
    {"No IP", "Pas d'IP"},
    {"Connected", "Connecte"},
    {"Connection failed", "Echec de connexion"},
    {"No network found", "Reseau introuvable"},
    {"Authentication failed", "Echec de l'authentification"},
};

template <>
const char *g_textTable<Settings::AlarmMode>[][LanguageCount] =
{
    {"Off", "Non"},
    {"Gradual beeps", "Bips progressifs"},
    {"Loud beeps", "Bips forts"},
    {"Music", "Musique"},
};

template <>
const char *g_textTable<Settings::HourlyChimeMode>[][LanguageCount] =
{
    {"Off", "Non"},
    {"Beep", "Bip"},
    {"If day: Beep", "Si jour: Bip"},
    {"Sound effect" , "Effet sonore"},
    {"If day: sound effect", "Si jour: effet sonore"},
};

template <>
const char *g_textTable<Settings::DateFormat>[][LanguageCount] =
{
    {"MM-DD", "MM-JJ"},
    {"MM/DD", "MM/JJ"},
    {"DD-MM", "JJ-MM"},
    {"DD/MM", "JJ/MM"},
    {"DD.MM", "JJ.MM"},
};

template <>
const char *g_textTable<Settings::SyncSource>[][LanguageCount] =
{
    {"RTC", "RTC"},
    {"NTP", "NTP"},
    {"GPS", "GPS"},
};
