
#import <UIKit/UIKit.h>

#ifdef __cplusplus
extern "C" {
#endif

void __ios_log_print(const char *format, ... )
{
    char buffer[500];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    NSLog(@"%s", buffer);
}

#ifdef __cplusplus
}
#endif