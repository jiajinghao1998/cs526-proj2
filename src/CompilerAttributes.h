#ifndef COMPILER_ATTRIBUTE_H
#define COMPILER_ATTRIBUTE_H
/*
 * Ref: https://elixir.bootlin.com/linux/latest/source/include/linux/compiler_attributes.h#L209
 * Add the pseudo keyword 'fallthrough' so case statement blocks
 * must end with any of these keywords:
 *   break;
 *   fallthrough;
 *   continue;
 *   goto <label>;
 *   return [expression];
 *
 *  gcc: https://gcc.gnu.org/onlinedocs/gcc/Statement-Attributes.html#Statement-Attributes
 */
#if __has_attribute(__fallthrough__)
# define __fallthrough __attribute__((__fallthrough__))
#else
# define __fallthrough do {} while (0)  /* fallthrough */
#endif

#if __cplusplus >= 201703L
# define fallthrough [[fallthrough]]
#else
# define fallthrough __fallthrough
#endif

#endif /* COMPILER_ATTRIBUTE_H */
