/* 
 *  Templated log function to serial out
 *  (Can be modifiet to print to stdout, ...)
 *  Usage: LOG("xyz", ...);
 */
template <typename T>
void LOG(T &&t)
{
    Serial.print(t);
    Serial.print("\r\n");
}
template <typename Head, typename... Tail>
void LOG(Head &&head, Tail &&... tail)
{
    Serial.print(head);
    LOG(std::forward<Tail>(tail)...);
}

//Returns stringified hex (without 0x prefix)
String toHex(uint32_t num)
{
    char hex[8+1];
    sprintf(hex, "%x", num);  
    return hex;
}
