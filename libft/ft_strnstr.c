/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 16:25:38 by rluiz             #+#    #+#             */
/*   Updated: 2023/04/16 18:12:08 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *str, const char *sub, size_t len)
{
	size_t	sublen;
	int		i;

	if (!*sub)
		return ((char *)str);
	sublen = ft_strlen(sub);
	i = 0;
	while (str[i] && i + sublen <= len)
	{
		if (ft_strncmp(&str[i], sub, sublen) == 0)
			return ((char *)&str[i]);
		i++;
	}
	return (NULL);
}
