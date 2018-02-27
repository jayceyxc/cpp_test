# 基于rsync的简易的发布测试和部署的脚本
# rsync options:
# --delete 同步时，目的目录中的、不在源目录中的文件和目录将被直接删除
# --dry-run 只进行模拟运行，不实际执行

# --no-perms  不比较权限，不改动权限
# --no-owner  不比较所有者，不改动所有者
# --no-group  不比较所有者组，不改动所有者组

function sync_test() {
    echo "将本地的改动发布测试服务器进行测试"
    echo "将代码同步至测试服务器"
    rsync -avr --delete --no-perms  --no-owner  --no-group --exclude-from=exclude_me.txt ./*  dev2:CPPSource/cpp_test
}

function publish() {
    echo "从部署服务器本地取得SVN更新，之后发布部署"
    cd /root/thinkphp_aoa_pre
    svn up
    chmod -R 777 .
    rsync -avr --exclude-from=exclude_me.txt ./*  /data/www/thinkphp_aoa_pre
}

sync_test
