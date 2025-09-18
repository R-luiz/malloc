/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/16 16:44:12 by rluiz             #+#    #+#             */
/*   Updated: 2023/04/16 16:50:45 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	count_words(char const *s, char c)
{
	int	count;
	int	is_word;

	count = 0;
	is_word = 0;
	while (*s)
	{
		if (*s != c && !is_word)
		{
			is_word = 1;
			count++;
		}
		else if (*s == c)
			is_word = 0;
		s++;
	}
	return (count);
}

static char	*get_next_word(char **s, char c)
{
	char	*start;
	char	*end;

	while (**s == c)
		(*s)++;
	start = *s;
	while (**s && **s != c)
		(*s)++;
	end = *s;
	return (ft_substr(start, 0, end - start));
}

char	**ft_split(char const *s, char c)
{
	char	**result;
	char	*str;
	int		words_count;
	int		i;

	if (!s)
		return (NULL);
	str = (char *)s;
	words_count = count_words(s, c);
	result = (char **)malloc((words_count + 1) * sizeof(char *));
	if (!result)
		return (NULL);
	i = 0;
	while (i < words_count)
	{
		result[i] = get_next_word(&str, c);
		i++;
	}
	result[i] = NULL;
	return (result);
}
