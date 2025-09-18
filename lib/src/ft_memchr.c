/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 15:41:58 by rluiz             #+#    #+#             */
/*   Updated: 2023/04/14 16:06:34 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memchr(const void *s, int c, int n)
{
	const unsigned char	*ptr = (const unsigned char *)s;

	while (n--)
	{
		if (*ptr == (unsigned char)c)
			return ((void *)ptr);
		ptr++;
	}
	return (NULL);
}
