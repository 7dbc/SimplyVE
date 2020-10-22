package commonItem 

import (
    // "os"
    // "bytes"
    "bufio"
    "strconv"
    "strings"
    "goprobe/model"
    "github.com/codeskyblue/go-sh"
    log "github.com/cihub/seelog"
)

func MonitorCommonItem(flag bool) {
    MonitorCPU(flag)
    MonitorMemory(flag)
    // MonitorDisk()
} 

func MonitorCPU(flag bool) {
    log.Info("Monitor CPU start!")
    coreNum := -1 
    session := sh.NewSession()
    out, err := session.Command("cat", "/proc/stat").Output()
    if err != nil {
        panic(err)
    }
    var arrCPU []model.CPURaw
    scanner := bufio.NewScanner(strings.NewReader(string(out)))
    for scanner.Scan() {
        line :=  scanner.Text()
        fields := strings.Fields(line)
        if len(fields) > 0 && strings.Contains(fields[0], "cpu") {
            var cpuitme model.CPURaw
            coreNum++
            cpuitme.CPUNum = string(fields[0])
            parseCPUFields(fields, &cpuitme)
            arrCPU = append(arrCPU, cpuitme)
        }
    }

    if !flag {
        model.PreCPU = arrCPU
        return
    }
    var arrCPUInfo []model.CPUInfo
    for i := 0; i < len(arrCPU); i++ {
        stat := arrCPU[i]
        var cpuitem model.CPUInfo
        cpuitem.CPUNum = stat.CPUNum
        total := float32(stat.Total - model.PreCPU[i].Total)
        cpuitem.User = float32(stat.User -  model.PreCPU[i].User) / total * 100
        cpuitem.Nice = float32(stat.Nice -  model.PreCPU[i].Nice) / total * 100
        cpuitem.System = float32(stat.System -  model.PreCPU[i].System) / total * 100
        cpuitem.Idle = float32(stat.Idle -  model.PreCPU[i].Idle) / total * 100
        arrCPUInfo = append(arrCPUInfo, cpuitem)
    }
    model.PreCPU = arrCPU

    log.Info("The number of terminal cpu core is ", coreNum,)
    for l := 0; l < len(arrCPUInfo); l++ {
        cpuInfo := arrCPUInfo[l]
        log.Info("CPU Information:")
        log.Info("--------------------------------")
        log.Info("CPU Name: ", cpuInfo.CPUNum)
        log.Info("User: ", cpuInfo.User)
        log.Info("Nice: ", cpuInfo.Nice)
        log.Info("System: ", cpuInfo.System)
        log.Info("Idle: ", cpuInfo.Idle)
        log.Info("--------------------------------")
    }
    log.Info("Monitor CPU end!")
}

func parseCPUFields(fields []string, stat *model.CPURaw) {
    numFields := len(fields)
    for i := 1; i < numFields; i++ {
        val, err := strconv.ParseUint(fields[i], 10, 64)
        if err != nil {
            continue
        }
        stat.Total += val
        switch i {
            case 1:
                stat.User = val
            case 2:
                stat.Nice = val
            case 3:
                stat.System = val
            case 4:
                stat.Idle = val
            case 5:
                stat.Iowait = val
            case 6:
                stat.Irq = val
            case 7:
                stat.SoftIrq = val
            case 8:
                stat.Steal = val
            case 9:
                stat.Guest = val
        }
    }
}

func MonitorMemory(flag bool) {
    log.Info("Monitor Memory start!")
    session := sh.NewSession()
    lines, err := session.Command("cat", "/proc/meminfo").Output()
    if err != nil {
        panic(err)
    }

    var stats model.MemoryInfo
    scanner := bufio.NewScanner(strings.NewReader(string(lines)))
    for scanner.Scan() {
        line := scanner.Text()
        parts := strings.Fields(line)
        if len(parts) == 3 {
            val, err := strconv.ParseUint(parts[1], 10, 64)
            if err != nil {
                continue
            }
        val *= 1024
        switch parts[0] {
            case "MemTotal:":
                    stats.MemTotal = val
            case "MemFree:":
                    stats.MemFree = val
            case "Buffers:":
                    stats.MemBuffers = val
            case "Cached:":
                    stats.MemCached = val
            case "SwapTotal:":
                    stats.SwapTotal = val
            case "SwapFree:":
                    stats.SwapFree = val
            }
        }
    }
    log.Info("Memory Information:")
    log.Info("--------------------------------")    
    log.Info("Total: ",      stats.MemTotal)
    log.Info("Free: ",       stats.MemFree)
    log.Info("Buffer: ",     stats.MemBuffers)
    log.Info("Cached: ",     stats.MemCached)
    log.Info("Swap total: ", stats.SwapTotal)
    log.Info("Swap free: ",  stats.SwapFree)
    log.Info("--------------------------------")    
    log.Info("Monitor Memory end!")
}
