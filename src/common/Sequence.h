//---------------------------------------------------------------------------
//
//      S c r i p t e d   S e q u e n c e s
//
//---------------------------------------------------------------------------

#pragma once

#include "Platform.h"

//---------------------------------------------------------------------------
// client_textmessage_t
//---------------------------------------------------------------------------
struct client_textmessage_t
{
    int effect;
    byte r1, g1, b1, a1; // 2 colors for effects
    byte r2, g2, b2, a2;
    float x;
    float y;
    float fadein;
    float fadeout;
    float holdtime;
    float fxtime;
    const char* pName;
    const char* pMessage;
};


//--------------------------------------------------------------------------
// sequenceDefaultBits_e
//
// Enumerated list of possible modifiers for a command.  This enumeration
// is used in a bitarray controlling what modifiers are specified for a command.
//---------------------------------------------------------------------------
enum sequenceModifierBits_e
{
    SEQUENCE_MODIFIER_EFFECT_BIT = (1 << 1),
    SEQUENCE_MODIFIER_POSITION_BIT = (1 << 2),
    SEQUENCE_MODIFIER_COLOR_BIT = (1 << 3),
    SEQUENCE_MODIFIER_COLOR2_BIT = (1 << 4),
    SEQUENCE_MODIFIER_FADEIN_BIT = (1 << 5),
    SEQUENCE_MODIFIER_FADEOUT_BIT = (1 << 6),
    SEQUENCE_MODIFIER_HOLDTIME_BIT = (1 << 7),
    SEQUENCE_MODIFIER_FXTIME_BIT = (1 << 8),
    SEQUENCE_MODIFIER_SPEAKER_BIT = (1 << 9),
    SEQUENCE_MODIFIER_LISTENER_BIT = (1 << 10),
    SEQUENCE_MODIFIER_TEXTCHANNEL_BIT = (1 << 11),
};


//---------------------------------------------------------------------------
// sequenceCommandEnum_e
//
// Enumerated sequence command types.
//---------------------------------------------------------------------------
enum sequenceCommandEnum_e
{
    SEQUENCE_COMMAND_ERROR = -1,
    SEQUENCE_COMMAND_PAUSE = 0,
    SEQUENCE_COMMAND_FIRETARGETS,
    SEQUENCE_COMMAND_KILLTARGETS,
    SEQUENCE_COMMAND_TEXT,
    SEQUENCE_COMMAND_SOUND,
    SEQUENCE_COMMAND_GOSUB,
    SEQUENCE_COMMAND_SENTENCE,
    SEQUENCE_COMMAND_REPEAT,
    SEQUENCE_COMMAND_SETDEFAULTS,
    SEQUENCE_COMMAND_MODIFIER,
    SEQUENCE_COMMAND_POSTMODIFIER,
    SEQUENCE_COMMAND_NOOP,

    SEQUENCE_MODIFIER_EFFECT,
    SEQUENCE_MODIFIER_POSITION,
    SEQUENCE_MODIFIER_COLOR,
    SEQUENCE_MODIFIER_COLOR2,
    SEQUENCE_MODIFIER_FADEIN,
    SEQUENCE_MODIFIER_FADEOUT,
    SEQUENCE_MODIFIER_HOLDTIME,
    SEQUENCE_MODIFIER_FXTIME,
    SEQUENCE_MODIFIER_SPEAKER,
    SEQUENCE_MODIFIER_LISTENER,
    SEQUENCE_MODIFIER_TEXTCHANNEL,
};


//---------------------------------------------------------------------------
// sequenceCommandType_e
//
// Typeerated sequence command types.
//---------------------------------------------------------------------------
enum sequenceCommandType_e
{
    SEQUENCE_TYPE_COMMAND,
    SEQUENCE_TYPE_MODIFIER,
};


//---------------------------------------------------------------------------
// sequenceCommandMapping_s
//
// A mapping of a command enumerated-value to its name.
//---------------------------------------------------------------------------
struct sequenceCommandMapping_s
{
    sequenceCommandEnum_e commandEnum;
    const char* commandName;
    sequenceCommandType_e commandType;
};


//---------------------------------------------------------------------------
// sequenceCommandLine_s
//
// Structure representing a single command (usually 1 line) from a
//  .SEQ file entry.
//---------------------------------------------------------------------------
struct sequenceCommandLine_s
{
    int commandType;                        // Specifies the type of command
    client_textmessage_t clientMessage;     // Text HUD message struct
    char* speakerName;                      // Targetname of speaking entity
    char* listenerName;                     // Targetname of entity being spoken to
    char* soundFileName;                    // Name of sound file to play
    char* sentenceName;                     // Name of sentences.txt to play
    char* fireTargetNames;                  // List of targetnames to fire
    char* killTargetNames;                  // List of targetnames to remove
    float delay;                            // Seconds 'till next command
    int repeatCount;                        // If nonzero, reset execution pointer to top of block (N times, -1 = infinite)
    int textChannel;                        // Display channel on which text message is sent
    int modifierBitField;                   // Bit field to specify what clientmessage fields are valid
    sequenceCommandLine_s* nextCommandLine; // Next command (linked list)
};


//---------------------------------------------------------------------------
// sequenceEntry_s
//
// Structure representing a single command (usually 1 line) from a
//  .SEQ file entry.
//---------------------------------------------------------------------------
struct sequenceEntry_s
{
    char* fileName;                      // Name of sequence file without .SEQ extension
    char* entryName;                     // Name of entry label in file
    sequenceCommandLine_s* firstCommand; // Linked list of commands in entry
    sequenceEntry_s* nextEntry;          // Next loaded entry
    qboolean isGlobal;                   // Is entry retained over level transitions?
};


//---------------------------------------------------------------------------
// sentenceEntry_s
// Structure representing a single sentence of a group from a .SEQ
// file entry.  Sentences are identical to entries in sentences.txt, but
// can be unique per level and are loaded/unloaded with the level.
//---------------------------------------------------------------------------
struct sentenceEntry_s
{
    char* data;                 // sentence data (ie "We have hostiles" )
    sentenceEntry_s* nextEntry; // Next loaded entry
    qboolean isGlobal;          // Is entry retained over level transitions?
    unsigned int index;         // this entry's position in the file.
};


//--------------------------------------------------------------------------
// sentenceGroupEntry_s
// Structure representing a group of sentences found in a .SEQ file.
// A sentence group is defined by all sentences with the same name, ignoring
// the number at the end of the sentence name.  Groups enable a sentence
// to be picked at random across a group.
//--------------------------------------------------------------------------
struct sentenceGroupEntry_s
{
    char* groupName;                 // name of the group (ie CT_ALERT )
    unsigned int numSentences;       // number of sentences in group
    sentenceEntry_s* firstSentence;  // head of linked list of sentences in group
    sentenceGroupEntry_s* nextEntry; // next loaded group
};


//---------------------------------------------------------------------------
// Function declarations
//---------------------------------------------------------------------------
sequenceEntry_s* SequenceGet(const char* fileName, const char* entryName);
void Sequence_ParseFile(const char* fileName, qboolean isGlobal);
void Sequence_OnLevelLoad(const char* mapName);
sentenceEntry_s* SequencePickSentence(const char* groupName, int pickMethod, int* picked);
