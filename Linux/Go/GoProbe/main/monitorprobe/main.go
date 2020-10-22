package main

import (
//    "fmt"
    "os"
//    "os/exec"
    "os/signal"
//    "sync"
    "syscall"
    "time"
    "runtime"
    log "github.com/cihub/seelog"
    "goprobe/model"
    "goprobe/common-item"
    "goprobe/custom-stats"
    "goprobe/network-traffic"
)

func main() {
    defer func() {
        if err := recover(); err != nil {
            log.Errorf("error in go routine. \nerror: %s", err)
        }
        log.Flush()
    }()
 
    configureLogger()
    interval := time.Duration(30) * time.Second
    sig := make(chan os.Signal, 1)
    signal.Notify(sig, os.Interrupt, syscall.SIGTERM, syscall.SIGQUIT) 
    timer := time.Tick(interval)
    done := false
    var traffics = make(map[string]model.Traffic)
    commonItem.MonitorCommonItem(false)
    customstats.MonitorCustomStats(false)
    network.MonitorNetworkTraffic(traffics, false)
    for !done {
        select {
            case <-sig:
                done = true
                // leave 1MB space for maintaining the scene
                buf := make([]byte, 1<<20)
                runtime.Stack(buf, true)
                log.Info("=== goroutine stack trace...\n")
                log.Info(string(buf))
                log.Info("\n end")
                log.Info("exit!")
            case <-timer:
                commonItem.MonitorCommonItem(true)
                customstats.MonitorCustomStats(true)
                network.MonitorNetworkTraffic(traffics, true)
        }
    }
}


func configureLogger() {
    testConfig := `
<seelog>
    <outputs formatid="main">
            <rollingfile type="size" filename="./log/goprobe.log" maxsize="10000000" maxrolls="5" />
    </outputs>
    <formats>
            <format id="main" format="%Date/%Time [%LEV] %Msg%n"/>
    </formats>
</seelog>
`
    logger, _ := log.LoggerFromConfigAsBytes([]byte(testConfig))
    log.ReplaceLogger(logger)
    log.Info("complete log configuration")
}
