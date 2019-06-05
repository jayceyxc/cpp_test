#include <string>

extern "C" {
    #include "friso.h"
}

using namespace std;

int main(int argc, char* argv[]) {
    //创建 friso_t
    friso_t friso = friso_new();

    //创建词库 dic(并没有加载词库)
    friso_config_t config = friso_new_config();

    //从指定的 friso.ini 配置文件中初始化 friso 和 config
    //__path__为 friso.ini 配置文件的地址
    fstring __path__ = "conf/friso.ini";

    //成功返回 1, 失败返回 0
    friso_init_from_ifile(friso, config, __path__);

    //自定义切分模式(简易，复杂，检测模式)
    //__FRISO_SIMPLE_MODE__
    //__FRISO_COMPLEX_MODE__
    //__FRISO_DETECT_MODE__
    friso_set_mode(config, __FRISO_DETECT_MODE__);

    //创建一个分词任务实例
    friso_task_t task = friso_new_task();

    //给分词任务设置分词的内容.
    fstring text = "研究生命起源";
    friso_set_text( task, text );

    //获取切分结果
    while ( ( config->next_token( friso, config, task ) ) != NULL ) {
        //查看 friso_token_t 可以获取更多信息。
        //printf("%s[%d]/ ", task->token->word, task->token->offset );
        printf("%s/ ", task->token->word );
    }


    //释放 friso_task_t 实例
    friso_free_task( task );

    //释放 friso_t 实例
    friso_free(friso);
    friso_free_config(config);
}
