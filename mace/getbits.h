/*
 * Giovanni Agosta, Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * getbits.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */
#ifndef _GETBITS_H
#define _GETBITS_H
/* Extracts bits from data. Lowest bit is `from`, highest bit is `to`. */
unsigned int getbits(unsigned int data, int from, int to);
#endif /* _GETBITS_H */
