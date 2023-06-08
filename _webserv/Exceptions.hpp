/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:37:05 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/08 12:34:08 by earendil         ###   ########.fr       */
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

class ConfigFileException: public std::exception
{
    private:
        ConfigFileException();
    public:
        const std::string _message;
        ConfigFileException(const std::string & s) : _message(s){}
        virtual const char * what () const throw(){return (("Configuration file error: " + _message + ".").c_str());}
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