#ifndef NUMBERPARAMETER_H
#define NUMBERPARAMETER_H

#include <string>     // std::string, std::stod
#include "config/configparameterbase.h"

template < typename T >
/**
 * @brief Represents a scalar parameter that can be of any generic type capable of being parsed from a string
 * using stream insertion operator of the stringstream class.
 */
class SingleParameter : public ConfigParameterBase
{

public:

    SingleParameter(const string key, const string title, const string units, const ParameterValidator * validator, T * const data)
        : ConfigParameterBase(key, title, units, validator), data(data) {

//        entryField = new QLineEdit();
//        entryField->setReadOnly(false);
    }

    // Pointer to the corresponding parameter in the state object, which is set whenever the parameter is
    // read from the GUI/config file and validated successfully
    T * const data;

//    void parseAndValidate() {

//        QString entry = entryField->text();
//        QByteArray ba = entry.toLatin1();
//        const char *c_entry = ba.data();

//        parseAndValidate(c_entry);

//        // Check if the parameter is valid and incidate in the form if not
//        switch(isValid) {
//        case VALID: {
//            // Clear any red borders for previously invalid parameters
//            entryField->setStyleSheet("");
//            entryField->setToolTip(tr(""));
//            break;
//        }
//        case WARNING: {
//            entryField->setStyleSheet("QLineEdit{border: 2px solid yellow}");
//            entryField->setToolTip(tr(par->message.c_str()));
//            break;
//        }
//        case INVALID: {
//            // Indicate on the form by turning borders red
//            entryField->setStyleSheet("QLineEdit{border: 2px solid red}");
//            entryField->setToolTip(tr(par->message.c_str()));
//            break;
//        }
//        }
//    }

    void parseAndValidate(const string stringRep) {

        value = stringRep;

        // Parse generic type parameter.
        // See https://stackoverflow.com/questions/7254009/how-to-convert-a-string-to-a-templated-type-in-c
        T parsed;
        std::stringstream  linestream(stringRep);
        linestream >> parsed;

        // If the linestream is bad, then reading the key/value failed
        // If reading one more `char` from the linestream works then there is extra crap in the line
        // thus we have bad data on a line.
        if (!linestream || !linestream.eof())
        {
            // Error
            isValid = INVALID;
            message = "Couldn't parse "+this->key+" from string \'" + stringRep + "\'";
            return;
        }

        // Now perform any additional tests on the value itself
        std::ostringstream strs;

        if(!validator->validate(&parsed, strs)) {
            message = strs.str();
            isValid = INVALID;
            return;
        }

        // All good: update the value of the state field
        (*data) = parsed;
        isValid = VALID;
        message = "";
        return;
    }

};

#endif // NUMBERPARAMETER_H
