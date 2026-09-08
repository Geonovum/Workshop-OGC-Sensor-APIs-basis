String formatString(const char *fmt, ...) {
    char buf[128];  // adjust size for your needs
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return String(buf);
}