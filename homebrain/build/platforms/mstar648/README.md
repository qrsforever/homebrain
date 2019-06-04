
打包文件名规范:
    tag_目标平台_日期.tar.gz

例如:
    homebrain_mstar648_20180910205041.tar.gz


样本间安装:

1. 主机为linux平台
   <1> 解压homebrain_mstar648_20180910205041.tar.gz
   <2> cd homebrain_mstar648_20180910205041/root
   <3> 修改adb_push_package中BOX_IP (目标机的IP)
   <4> ./adb_push_package.sh


2. 主机为Windows平台:
   <1> 解压homebrain_mstar648_20180910205041.tar.gz
   <2> 修改root/data/homebrain$/hue_bridge.conf
       A. 浏览器输入: https://www.meethue.com/api/nupnp
       B. 返回结果, 类似[{"id":"001788fffe751108","internalipaddress":"192.168.124.4"}]
       C. 使用上面的结果替换hue_bridge.conf中的id和internalipaddress对应的值
       D. 修改username的值:
              如果id为001788fffe751108, username的值改为: 2RU3MJROCn32Hn6OqJLg3IMzYTIUS9iruwNsH34C
              如果id为001788fffe755132, username的值改为: sSJxR6KqKcctgaXQCGJRS0UxRNPkvQhF6GADztDp
   <3> 删除盒子里的/data/homebrain目录 (adb 相关命令: adb shell rm -r /data/homebrain)
   <4> 使用adb或者U盘的方式: (adb相关命令: adb connect, adb root, adb connect, adb remount, adb push, adb reboot)
       将root/data/homebrain拷贝到/data/
       将root/system/lib64拷贝到/system/lib64/
       将root/sysetm/bin/拷贝到/system/bin
   <5> 重启盒子
