package model

/***************************************************************************
CPU MODEL
***************************************************************************/
type CPURaw struct {
    CPUNum  string // cpu NO. for multi-core
    User    uint64 //time spent in user mode
    Nice    uint64 //time spent in user mode with low priority (nice)
    System  uint64 //time spent in system mode
    Idle    uint64 //time spent in the idle task
    Iowait  uint64 //time spent waiting for I/O to complete (since Linux 2.5.41)
    Irq     uint64 //time spent servicing  interrupts  (since  2.6.0-test4)
    SoftIrq uint64 //time spent servicing softirqs (since 2.6.0-test4)
    Steal   uint64 //time spent in other OSes when running in a virtualized environment
    Guest   uint64 //time spent running a virtual CPU for guest operating systems under the control of the Linux kernel.
    Total   uint64 //total of all time fields
}

type CPUInfo struct {
    CPUNum  string 
    User    float32
    Nice    float32
    System  float32
    Idle    float32
    Iowait  float32
    Irq     float32
    SoftIrq float32
    Steal   float32
    Guest   float32
}

var PreCPU []CPURaw


/***************************************************************************
Memory MODEL
***************************************************************************/
type MemoryInfo struct {
    MemTotal     uint64
    MemFree      uint64
    MemBuffers   uint64
    MemCached    uint64
    SwapTotal    uint64
    SwapFree     uint64
}
