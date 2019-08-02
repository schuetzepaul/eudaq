#!/usr/bin/env sh
BINPATH=../../../bin
#$BINPATH/euRun &
$BINPATH/euRun -n Ex0RunControl &
sleep 1
$BINPATH/euLog &
sleep 1
$BINPATH/euCliProducer -n QDCProducer -t QDC &
sleep 1
$BINPATH/euCliCollector -n TriggerIDSyncDataCollector -t dc &
sleep 1
$BINPATH/StdEventMonitor -t StdEventMonitor &
