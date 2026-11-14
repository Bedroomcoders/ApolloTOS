/*
 * apollologo.h - 480x120 4bpp Atari planar boot logo for ApolloTOS
 *
 * Generated from extras/ApolloTOS-480x120.png
 */

#ifndef APOLLOLOGO_H
#define APOLLOLOGO_H

#if CONF_WITH_APOLLO_V4

#define APOLLOLOGO_WIDTH   480
#define APOLLOLOGO_HEIGHT  120
#define APOLLOLOGO_PLANES  4

extern const ULONG apollologo_palette[16];
extern const UWORD apollologo_data[14400];

#endif /* CONF_WITH_APOLLO_V4 */

#endif /* APOLLOLOGO_H */