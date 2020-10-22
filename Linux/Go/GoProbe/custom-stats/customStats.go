package customstats 

import (
    "bytes"
    "bufio"
    "regexp"
    "strconv"
    "strings"
    "github.com/codeskyblue/go-sh"
    log "github.com/cihub/seelog"
)

func MonitorCustomStats(flag bool) {
    urlarr := []string{"www.baidu.com", "www.sina.com.cn"}
    MonitorNetAvaliable(urlarr)
}

func MonitorNetAvaliable(networkconn []string) {
    log.Info("MonitorNetAvaliable start!")
    var avgloss float64 = 0.0
    arrlen := len(networkconn)
    session := sh.NewSession()

    for _, item := range networkconn {
        out, err := session.Command("ping", "-c 10", "-w 100", item).Output()
        if err != nil {
            panic(err)
        }

        regexpat := "packet loss"
        scanner := bufio.NewScanner(strings.NewReader(string(out)))
        for scanner.Scan() {
            line := scanner.Text()
            matchresult, _ := regexp.MatchString(regexpat, string(line))
            if matchresult {
                parts := strings.Fields(line)
                tmplost := parts[5]
                tmpitem := string(bytes.TrimRight([]byte(tmplost), "%"))
                valitem, _ := strconv.ParseUint(tmpitem, 10, 32)
                avgloss += float64(valitem)
            }
        }     
    }

    netavaliable := avgloss / float64(arrlen)
    netavaliableDetails := "The network is avaliable to other terminal, and the package lost rate is " + strconv.FormatFloat(netavaliable, 'f', 2, 32) + "%"
    log.Info("NetAvaliable info for ", networkconn)
    log.Info("--------------------------------")
    log.Info("Average of packet lost: ", netavaliable, "%")
    log.Info("Network avaliable details: ", netavaliableDetails)
    log.Info("--------------------------------")
    
    log.Info("MonitorNetAvaliable end!")
}
