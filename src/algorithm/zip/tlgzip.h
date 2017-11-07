/***********************************************************************************************
// 文件名:     tlgzip.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012-2-22 23:05:01
// 内容描述:   支持gzip格式数据的压缩解压
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_TLGZIP_H
#define BASIC_TLGZIP_H

__NS_BASIC_START

/*!***********************************************************************
 * \function BASIC_gcompress
 * \param	dest[out] 输出的目标地址
 * \param	destLen[in/out]	in输出的目标块的长度。out压缩后数据的长度
 * \param	source[in] 需要压缩的数据块
 * \param	level[in] 压缩程度(1-9)
 * \param	bufOut[out] 输出
 * \remrk	dest的长度至少为：
			(sourceLen + 12) * 1.1 + 18
************************************************************************/
int basic_gcompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen, int level);

/*!***********************************************************************
* \function BASIC_guncompress
* \param	dest[out] 输出的目标地址
* \param	destLen[in/out]	in输出的目标块的长度。out解压后数据的长度
* \param	source[in] 需要解压缩的数据块
* \param	bufOut[out] 输出
* \remrk	当dest传入为NULL时，destLen的输出为解压后的长度
************************************************************************/
int basic_guncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);

__NS_BASIC_END

#endif 


