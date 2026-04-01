#include "ChordEngine/Fretboard.h"
#include "ChordEngine/Chord.h"
#include "ChordEngine/ChordHashTable.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace ChordEngine;

static void printUsage(const char* programName) {
    std::cout
        << "Usage: " << programName << " [options] [chord ...]\n\n"
        << "If chord names are provided, prints voicings for each and exits.\n"
        << "If none are provided, starts an interactive prompt.\n\n"
        << "Options:\n"
        << "  -h, --help              Show this help message and exit\n"
        << "  --tuning <name>         Guitar tuning: standard (default), dropd\n"
        << "  --max-voicings <N>      Max voicings to display per chord (default: 10)\n"
        << "  --max-span <N>          Max fret span for playability (default: 4)\n"
        << "  --min-strings <N>       Min strings sounded (default: 4)\n"
        << "  --no-root-bass          Don't require root note in the bass\n\n"
        << "Examples:\n"
        << "  " << programName << " Am G7 Cmaj7\n"
        << "  " << programName << " --tuning dropd --max-voicings 5 Em\n"
        << "  " << programName << "                   (starts interactive mode)\n";
}

static Fretboard buildFretboard(const std::string& tuningName) {
    if (tuningName == "dropd")
        return Fretboard::dropD();
    return Fretboard::standard();
}

// Parse, generate and print voicings for one chord symbol.
static bool processChord(
    const std::string& symbol,
    const Fretboard& fretboard,
    const PlayabilityConfig& config,
    int maxVoicings,
    std::ostream& outputStream)
{
    try {
        auto chord = Chord::fromSymbol(symbol);
        auto voicings = chord.generateVoicings(fretboard, config);

        int displayCount = std::min(static_cast<int>(voicings.size()), maxVoicings);
        outputStream << chord.getName() << " ("
                     << voicings.size() << " voicings, showing first "
                     << displayCount << "):\n";

        for (int voicingIdx = 0; voicingIdx < displayCount; ++voicingIdx) {
            outputStream << "  " << (voicingIdx + 1) << ". ";
            Chord::printVoicing(voicings[voicingIdx], outputStream);
        }
    } catch (const std::invalid_argument& error) {
        outputStream << "Error: " << error.what()
                     << " (\"" << symbol << "\")\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::string tuningName = "standard";
    int maxVoicings = 10;
    PlayabilityConfig config;
    std::vector<std::string> chordNames;

    for (int argIdx = 1; argIdx < argc; ++argIdx) {
        std::string arg = argv[argIdx];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--tuning" && argIdx + 1 < argc) {
            tuningName = argv[++argIdx];
        } else if (arg == "--max-voicings" && argIdx + 1 < argc) {
            maxVoicings = std::atoi(argv[++argIdx]);
        } else if (arg == "--max-span" && argIdx + 1 < argc) {
            config.maxFretSpan = std::atoi(argv[++argIdx]);
        } else if (arg == "--min-strings" && argIdx + 1 < argc) {
            config.minStringsSounded = std::atoi(argv[++argIdx]);
        } else if (arg == "--no-root-bass") {
            config.requireRootInBass = false;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        } else {
            chordNames.push_back(arg);
        }
    }

    Fretboard fretboard = buildFretboard(tuningName);

    // Argument mode
    if (!chordNames.empty()) {
        for (const std::string& chordName : chordNames) {
            processChord(chordName, fretboard, config, maxVoicings, std::cout);
            std::cout << "\n";
        }
        return 0;
    }

    // Interactive REPL
    std::cout << "ChordArchitect — interactive mode (type a chord name, or \"quit\" to exit)\n\n";
    std::string inputLine;
    while (true) {
        std::cout << "ChordArchitect> " << std::flush;
        if (!std::getline(std::cin, inputLine))
            break;
        if (inputLine.empty() || inputLine == "quit" || inputLine == "exit")
            break;

        processChord(inputLine, fretboard, config, maxVoicings, std::cout);
        std::cout << "\n";
    }

    return 0;
}
