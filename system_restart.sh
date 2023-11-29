#!/bin/bash
while [ 1 ] ; do
sleep 30
    if [ $(ps -ef|grep xian_retractable_control_system |grep -v grep|wc -l) -eq 0 ] ; then # 将exe_name替换成你想要监测的可执行程序名字
        sleep 1;
        echo "[`date +%F\ %T`] xian_retractable_control_system is offline, try to restart..." >> ./logs/check_es.log;
        /home/psa/aQC/retractable_control_system/build/xian_retractable_control_system &  # 将exe_name替换成你想要监测的可执行程序名字
    else
        echo "[`date +%F\ %T`] xian_retractable_control_system is online...";
    fi
done
