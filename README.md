Durable Transactions for Persistent Memory

This repository contains the source code and paper for multiple STMs capable of providing durable transactions in Persistent Memory (PM):

    ptms/quadra/QuadraFC.hpp        Quadra durability (1 fence)
    ptms/trinity/TrinityFC.hpp      Trinity durability (2 fences)
    ptms/redolog/RedoLogFC.hpp      Redo-Log (4 fences)
    ptms/redolog/RedoLog2FFC.hpp    Redo-Log with two logs (2 fences)
    ptms/redolog/RedoLog1FFC.hpp    Redo-Log with three logs (1 fence)
    ptms/romuluslog/RomLogFC.hpp    RomulusLog (4 fences)
    ptms/romuluslog/RomLog2FFC.hpp  RomulusLog 2 Fences (2 fences)
    ptms/romuluslog/RomLogTSFC.hpp  RomulusLog with hardware Time Stamp (2 fences)
    ptms/undolog/UndoLogFC.hpp      Undo-Log durability