package network

import (
    "os"
    "bytes"
    "bufio"
    "strconv"
    "strings"
    "goprobe/model"
    "github.com/codeskyblue/go-sh"
    log "github.com/cihub/seelog"
)

func MonitorNetworkTraffic(traffics map[string]model.Traffic, flag bool) {
    log.Info("MonitorNetworkTraffic start!")
    session := sh.NewSession()
    out, err := session.Command("ip", "-o", "-4", "addr", "list").Command("awk", "{print $2}").Output()
    if err != nil {
        panic(err)
    }

    scanner := bufio.NewScanner(strings.NewReader(string(out)))
    for scanner.Scan() {
        line :=  scanner.Text()
        if string(line) == "lo" {
            continue
        }
        // session.SetDir("/")
        pwd, _ := session.Command("pwd").Output()
        log.Info("work dir:", string(pwd))
        curAdapter := "/sys/class/net/" + string(line) + "/"
        log.Info("adapter path:", curAdapter)
        if session.Test("d", curAdapter) {
            log.Info("Adapter exsits!")
            rxbyte, _ := session.Command("cat", "/proc/net/dev").Command("grep", string(line)).Command("tr", ":", " ").Command("awk", "{print $2}").Output()
            rxbyte = bytes.TrimRight(rxbyte, "\n")
            txbyte, _ := session.Command("cat", "/proc/net/dev").Command("grep", string(line)).Command("tr", ":", " ").Command("awk", "{print $10}").Output()
            txbyte = bytes.TrimRight(txbyte, "\n")
            // log.Info("RX bytes: ", string(rxbyte))
            // log.Info("TX bytes: ", string(txbyte))
            rx_value, _ := strconv.ParseInt(string(rxbyte), 10, 64)
            tx_value, _ := strconv.ParseInt(string(txbyte), 10, 64)
            adaptername := string(line)
            hostname, _ := os.Hostname()
            if flag {
                tmp := traffics[adaptername]
                rx_delta := rx_value - tmp.PreRXBytes
                tx_delta := tx_value - tmp.PreTXBytes
                item := &model.Traffic{string(hostname), adaptername, tx_delta, rx_delta, tx_value, rx_value}
                traffics[adaptername] = (*item)
            } else {
                item := &model.Traffic{string(hostname), adaptername, 0, 0, tx_value, rx_value}
                traffics[adaptername] = (*item)
            }
        }
    } 

    for key, value := range traffics {
        log.Info("Key: ", key)
        log.Info("Value: ", value)
    }

    log.Info("MonitorNetworkTraffic end!")
}
