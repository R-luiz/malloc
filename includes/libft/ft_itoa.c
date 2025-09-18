/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/16 17:03:09 by rluiz             #+#    #+#             */
/*   Updated: 2023/04/16 17:19:30 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_numlen(int n)
{
	int	len;

	len = 0;
	if (n <= 0)
		len++;
	while (n != 0)
	{
		n /= 10;
		len++;
	}
	return (len);
}

static long	neg_case_helper(long num, char *str)
{
	if (num < 0)
	{
		str[0] = '-';
		num = -num;
	}
	return (num);
}

char	*ft_itoa(int n)
{
	char	*str;
	int		len;
	long	num;

	num = n;
	len = ft_numlen(n);
	str = (char *)malloc((len + 1) * sizeof(char));
	if (!str)
		return (NULL);
	str[len--] = '\0';
	if (num == 0)
	{
		str[0] = '0';
		return (str);
	}
	num = neg_case_helper(num, str);
	while (num != 0)
	{
		str[len--] = (num % 10) + '0';
		num /= 10;
	}
	return (str);
}
