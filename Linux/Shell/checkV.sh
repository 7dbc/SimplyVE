#!/bin/sh
#20161006
# check_vm.sh © 12309 at exploit.in
## TODO: 'wc','let' analogs !!!
## TODO: add more detects:
## - jail
## - chroot
## - cloudlinux ( .cagefs/.cagefs.enabled)
## - docker ( /.dockerenv ? dmesg|grep docker ?)
## ...

PATH=$PATH:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin;
export PATH=$PATH:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin 2>/dev/null;

# total amount of detects
vm=0;
# list of openvz detects
openvz="";
# list of xen detects
xen="";
# list of kvm detects
kvm="";
# list of vmware detects
vmware="";
# list of virtualbox detects
vbox="";
# list of hyper-v detects
hyperv="";
# list of other virtualisations detects
other="";

if [ -x "$(which expr 2>/dev/null)" ];
then 
  append() {
    vm=$(expr $vm + 1);
  }
else 
  append() {
    let vm=$vm+1; #TODO: /bin/dash cannot into 'let'
  }
fi;

dmidecode=$(dmidecode 2>&1);
echo "$dmidecode" | grep -q -i 'microsoft' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  hyperv="dmidecode";
fi;
echo "$dmidecode" | grep -q -i 'vmware' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="dmidecode";
fi;
echo "$dmidecode" | grep -q -i 'bochs' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="dmidecode";
fi;
echo "$dmidecode" | grep -q -i 'virtualbox' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vbox="dmidecode";
fi;
echo "$dmidecode" | grep -q -i 'xen' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  xen="dmidecode";
fi;

lsmod=$(lsmod 2>&1);
echo "$lsmod" | grep -q -E -i '(vmx|vmw)' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="lsmod $vmware";
fi;
echo "$lsmod" | grep -q -i 'xen' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  xen="lsmod $xen";
fi;
echo "$lsmod" | grep -q -i 'virtio' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="lsmod $kvm";
fi;
echo "$lsmod" | grep -q -i 'hv_' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  hyperv="lsmod $hyperv";
fi;
echo "$lsmod" | grep -q -E -i '(vze|vzm|vzd)' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  openvz="lsmod $openvz";
fi;

scsi=$(cat /proc/scsi/scsi 2>/dev/null);
echo "$scsi" | grep -q -i 'vmware' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="scsi $vmware";
fi;
echo "$scsi" | grep -q -i 'vbox' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vbox="scsi $vbox";
fi;
echo "$scsi" | grep -q -i 'qemu' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="scsi $kvm";
fi;

lspci=$(lspci 2>&1);
echo "$lspci" | grep -q -i 'virtio' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="lspci $kvm";
fi;
echo "$lspci" | grep -q -i 'vmware' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="lspci $vmware";
fi;
echo "$lspci" | grep -q -i 'virtualbox' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vbox="lspci $vbox";
fi;

lshw=$(lshw 2>&1);
echo "$lshw" | grep -q -E -i '(qemu|bochs|virtio)' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="lshw $kvm";
fi;
echo "$lshw" | grep -q -i 'vmware' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="lshw $vmware";
fi;
echo "$lshw" | grep -q -i 'virtualbox' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vbox="lshw $vbox";
fi;
#TODO: add more detects

lscpu=$(lscpu 2>&1);
echo "$lscpu" | grep -q -i 'vmware' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="lscpu $vmware";
fi;
echo "$lscpu" | grep -q -i 'kvm' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="lscpu $kvm";
fi;
echo "$lscpu" | grep -q -i 'xen' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  xen="lscpu $xen";
fi;
echo "$lscpu" | grep -q -i 'microsoft' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  hyperv="lscpu $hyperv";
fi;

dmesg=$(dmesg 2>&1);
echo "$dmesg" | grep -q -i 'vmware' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="dmesg $vmware";
fi;
echo "$dmesg" | grep -q -i 'vbox' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vbox="dmesg $vbox";
fi;
echo "$dmesg" | grep -q -E -i '(xen_m|xen_b|xen-v)' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  xen="dmesg $xen";
fi;
echo "$dmesg" | grep -q -i 'qemu' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="dmesg $kvm";
fi;

mount=$(mount 2>&1);
echo "$mount" | grep -q -i 'simfs' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  openvz="simfs $openvz";
fi;
echo "$mount" | grep -q -i 'xvda' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  xen="xvda $xen";
fi;
echo "$mount" | grep -q -i '/vda' 2>/dev/null;
if [ "$?" = "0" ];
then 
  append;
  kvm="vda $kvm";
fi;
echo "$mount" | grep -q -i 'vmblock' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="vmblock $vmware";
fi;

ifconfig=$(ifconfig 2>&1);
echo "$ifconfig" | grep -q -E -i '(venet|veth)' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  openvz="ifconfig $openvz";
fi;

psaux=$(ps aux 2>&1);
echo "$psaux" | grep -q -E '(\[virtio|\[vballo)' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="ps_aux $kvm";
fi;

interrupts=$(cat /proc/interrupts 2>/dev/null);
echo "$interrupts" | grep -q 'virtio' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="interrupts $kvm";
fi;
if [ -z "$interrupts" ];
then 
  append;
  openvz="no_interrupts $openvz";
fi;
#TODO: add more detects

cpuid=$(cpuid 2>/dev/null);
echo "$cpuid" | grep -i hypervisor_id | grep -q -i kvm 2>/dev/null;
if [ "$?" = "0" ];
then
  # virtualbox & kvm have same id "KVMKVMKVM "
  append;
  kvm="cpuid $kvm";
  vbox="cpuid $vbox";
fi;
echo "$cpuid" | grep -i hypervisor_id | grep -q -i vmware 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  vmware="cpuid $vmware";
fi;
#TODO: add more detects

if [ -d '/sys/bus/virtio/devices/' ];
then
  append;
  ls -1 /sys/bus/virtio/devices/ >/dev/null 2>/dev/null;
  if [ "$?" = "0" ];
  then
    kvm="bus $kvm";
  else
    vmware="bus $vmware";
  fi;
fi;

ls -1 /sys/bus | grep -q -i 'xen' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  xen="bus $xen";
fi;

if [ ! -d '/proc/bus' ] || [ ! -d '/sys/bus' ];
then
  append;
  openvz="no_bus $openvz";
fi;

if [ -s '/proc/user_beancounters' ];
then
  append;
  openvz="beancounters $openvz";
fi;

harddisk=$(ls /dev/disk/* 2>/dev/null);
echo "$harddisk" | grep -q -i 'virtio' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="harddisk $kvm";
fi
echo "$harddisk" | grep -q -i 'vbox' 2>/dev/null;
if [ "$?" = "0" ];
then 
  append;
  vbox="harddisk $vbox";
fi
#TODO: add more detects

cpuinfo=$(cat /proc/cpuinfo 2>/dev/null);
echo "$cpuinfo" | grep -q -i 'qemu' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  kvm="cpuinfo $kvm";
fi;
echo "$cpuinfo" | grep -q -i 'hypervisor' 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  other="hypervisor $other";
fi;
#TODO: add more detects

if uname -a 2>/dev/null | grep -q -i 'vz' 2>/dev/null;
then
  append;
  openvz="uname $openvz";
fi;
if uname -a 2>/dev/null | grep -q -i 'xen' 2>/dev/null;
then
  append;
  xen="uname $xen";
fi;

if [ -d '/proc/vz' ];
then
  append;
  openvz="proc $openvz";
fi;

if [ -d '/proc/xen' ];
then
  append;
  xen="proc $xen";
fi;

if grep -q 'xen' /sys/hypervisor/type 2>/dev/null;
then
  append;
  xen="hypervisor $xen";
fi;

if [ -h '/aquota.group' ] || [ -h '/aquota.user' ];
then
  append;
  openvz="quota $openvz";
fi;

hdd=$(df -l -P 2>/dev/null | grep -E -o "[0-9]{1,20}" 2>/dev/null | sort -n 2>/dev/null | tail -n 1 2>/dev/null);
if [ -z "$hdd" ];
then 
  ## no df? fallback to stat
  hdd=$(stat -f / 2>/dev/null | grep -i blocks 2>/dev/null | grep -E -o "[0-9]{1,20}" 2>/dev/null | sort -n 2>/dev/null | tail -n 1 2>/dev/null);
  if [ -z "$hdd" ] || [ "$hdd" -lt '2500000' ];
  then
    append;
    other="small_hdd $other";
  fi;
else 
  if [ "$hdd" -lt '60000000' ];
  then
    append;
    other="small_hdd $other";
  fi;
fi;
mem=$(grep -i 'memtotal' /proc/meminfo 2>/dev/null | grep -E -o "[0-9]{1,20}" 2>/dev/null);
if [ -z "$mem" ]; then mem=0; fi;
if [ "$mem" -lt '2000000' ];
then
  append;
  other="low_ram $other";
fi;
cpu=$(grep -i 'name' /proc/cpuinfo | wc -l); #TODO: wc analog!
if [ -z "$cpu" ]; then cpu=0; fi;
if [ "$cpu" -lt "2" ];
then
  append;
  other="single_core $other";
fi;
rez=$(echo "$psaux" 2>/dev/null | wc -l); #TODO: wc analog!
if [ "$rez" -lt "30" ];
then
  append;
  other="few_processes $other";
fi;
echo "$dmidecode" | grep -q "^\/dev\/mem" 2>/dev/null;
if [ "$?" = "0" ];
then
  append;
  other="no_dmidecode $other";
fi;

# -----------------------  full result  ----------------------------
echo '-----BEGIN OUTPUT-----';
if [ -n "$openvz" ] || [ -n "$xen" ] || [ -n "$vmware" ] || [ -n "$kvm" ] || [ -n "$vbox" ] || [ -n "$hyperv" ];
then
  echo "DETECTIONS: $vm";
  if [ -n "$openvz" ];
  then
    echo "openvz: $openvz";
  fi;
  if [ -n "$xen" ];
  then
    echo "xen: $xen";
  fi;
  if [ -n "$vmware" ];
  then
    echo "vmware: $vmware";
  fi;
  if [ -n "$kvm" ];
  then
    echo "kvm: $kvm";
  fi;
  if [ -n "$vbox" ];
  then
    echo "vbox: $vbox";
  fi;
  if [ -n "$hyperv" ];
  then
    echo "hyperv: $hyperv";
  fi;
fi;
if [ -n "$other" ];
then
  echo "other: $other";
fi;

# ---------------------- compact result -----------------------
echo;
openvz=$(echo $openvz|wc -w); #TODO: wc analog! vvvv
xen=$(echo $xen|wc -w);
vmware=$(echo $vmware|wc -w);
kvm=$(echo $kvm|wc -w);
vbox=$(echo $vbox|wc -w);
hyperv=$(echo $hyperv|wc -w);
other=$(echo $other|wc -w); #/TODO: wc analog! ^^^^
echo -e "openvz $openvz\nxen $xen\nvmware $vmware\nkvm $kvm\nvbox $vbox\nhyperv $hyperv\nother $other" | grep -v \ 0 | sort -n -k 2 2>/dev/null | tail -n 1 2>/dev/null;
echo '-----END OUTPUT-----';
