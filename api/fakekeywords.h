#ifndef fakekeys_h_seen
#define fakekeys_h_seen

static int fakekeys = 0; // expect whinage about unused.
// 
#define FECONNECT "CONNECT"
#define FEVERSION "VERSION"
#define FEFORMAT "FORMAT"

#ifdef FAKE_KEYS_FULL
// we don't care about logging performance in the small
// compile -DFAKE_KEYS_FULL to get these in output files.
// some of the macro names are prefixed with FE to avoid enum element conflicts.

#define DATACOLLECTION_COMMIT "DataCollection_commit"
#define DATACOLLECTION "DataCollection"
#define APPLICATION_COMMIT "Application_commit"
#define APPLICATION "Application"
#define DATASET_COMMIT "Dataset_commit"
#define DATASET "Dataset"
#define FEMACHINE_COMMIT "Machine_commit"
#define FEMACHINE "Machine"
#define TRIAL_COMMIT "Trial_commit"
#define TRIAL "Trial"
#define EXECUTION_COMMIT "Execution_commit"
#define EXECUTION "Execution"
#define MACHINEMETRIC_COMMIT "MachineMetric_commit"
#define MACHINEMETRIC "MachineMetric"
#define DETERMINISTICMETRIC_COMMIT "DeterministicMetric_commit"
#define DETERMINISTICMETRIC "DeterministicMetric"
#define NONDETERMINISTICMETRIC_COMMIT "NondeterministicMetric_commit"
#define NONDETERMINISTICMETRIC "NondeterministicMetric"
#define METRIC_COMMIT "Metric_commit"
#define FEMETRIC "Metric"
#define KWFORMAT "full"
#error "not here please"
#else
// shorter, faster, barely readable.
// caps are commits, lowercase are ctor.
// mnemonics: C-dataCollection, S-dataSet, R-hardware rate, H-hardware, V-value
#define DATACOLLECTION_COMMIT "C"
#define DATACOLLECTION "c"
#define APPLICATION_COMMIT "A"
#define APPLICATION "a"
#define DATASET_COMMIT "S"
#define DATASET "s"
#define FEMACHINE_COMMIT "H"
#define FEMACHINE "h"
#define TRIAL_COMMIT "T"
#define TRIAL "t"
#define EXECUTION_COMMIT "E"
#define EXECUTION "e"
#define MACHINEMETRIC_COMMIT "R"
#define MACHINEMETRIC "r"
#define DETERMINISTICMETRIC_COMMIT "D"
#define DETERMINISTICMETRIC "d"
#define NONDETERMINISTICMETRIC_COMMIT "N"
#define NONDETERMINISTICMETRIC "n"
#define METRIC_COMMIT "v"
#define FEMETRIC "V"
#define KWFORMAT "character"

#endif // FAKE_KEYS_FULL

#endif // fakekeys_h_seen
