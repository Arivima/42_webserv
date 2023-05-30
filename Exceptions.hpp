/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:37:05 by avilla-m          #+#    #+#             */
/*   Updated: 2023/05/30 12:47:06 by avilla-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

# include <iostream>
# include <string>
# include <stdexcept>

class SystemCallException: public std::exception
{
    private:
        SystemCallException();
    public:
        const std::string _sysCall;
        SystemCallException(const std::string & s) : _sysCall(s){}
        virtual const char * what () const throw(){return (("system call " + _sysCall + " failed.").c_str());}
};


//! EXAMPLE
// class BLABLAException: public std::exception
// {
// 	public:
// 		virtual const char * what () const throw()
// 		{
// 			return ("BLABLA Error_message");
// 		}
// };

#endif