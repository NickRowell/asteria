#ifndef PARAMETERMULTIPLECHOICE_H
#define PARAMETERMULTIPLECHOICE_H

#include "config/configparameterbase.h"

#include <string>

#include <QComboBox>

template < typename T >
/**
 * @brief Represents a scalar parameter that can be of any generic type capable of being parsed from a string
 * using stream insertion operator of the stringstream class. The parameter is chosen from a fixed set of options
 * defined on construction.
 */
class ParameterMultipleChoice : public ConfigParameterBase
{
public:

    /**
     * @brief Constructor for the ParameterMultipleChoice. This is a variant of the ConfigParameterBase in which the
     * parameter value is selected from among a preset list of options.
     * @param key
     *  The key used to identify the parameter in config file etc
     * @param title
     *  Descriptive name of the parameter
     * @param options
     *  The set of options
     * @param data
     *  Pointer to the corresponding field in the state object.
     */
    ParameterMultipleChoice(const string key, const string title, const std::vector<T> options, T * const data)
        : ConfigParameterBase(key, title, "", 0), data(data), options(options) {

        QComboBox * cb = new QComboBox();

        // Configure the combo box options
        cb->setEditable(false);
        cb->setFrame(true);

        for(unsigned int i=0; i<options.size(); i++) {
            std::stringstream ss;
            ss << options[i];
            cb->insertItem(i, QString::fromStdString(ss.str()));
        }

        entryField = cb;
    }

    /**
     * @brief Pointer to the corresponding parameter in the state object, which is set whenever the parameter is
     * read from the GUI/config file and validated successfully
     */
    T * const data;

    /**
     * @brief The available options.
     */
    const std::vector<T> options;

    void parseAndValidate() {
        // Read selected entry in the combo box and configure field
        QComboBox * cb = (QComboBox *)entryField;
        QString qs = cb->currentText();
        parseAndValidate(qs.toStdString());
    }

    void writeToGui() {

        // Pick out current parameter selection and indicate in GUI combo box
        QComboBox * cb = (QComboBox *)entryField;

        for(unsigned int optIdx = 0; optIdx < options.size(); optIdx++) {
            std::stringstream ss;
            ss << options[optIdx];
            if(ss.str().compare(value.c_str()) == 0) {
                cb->setCurrentIndex(optIdx);
            }
        }

        // Check if the parameter is valid and incidate in the form if not
        switch(isValid) {
            case VALID: {
                // Clear any red borders for previously invalid parameters
                cb->setStyleSheet("");
                cb->setToolTip(QObject::tr(""));
                break;
            }
            case WARNING: {
                cb->setStyleSheet("QComboBox{border: 2px solid yellow}");
                cb->setToolTip(QObject::tr(message.c_str()));
                break;
            }
            case INVALID: {
                // Indicate on the form by turning borders red
                cb->setStyleSheet("QComboBox{border: 2px solid red}");
                cb->setToolTip(QObject::tr(message.c_str()));
                break;
            }
        }
    }

    void parseAndValidate(const string option) {

        value = option;

        // Verify that the option exists in the set of options
        for(unsigned int optIdx = 0; optIdx < options.size(); optIdx++) {

            // To check that the given option (represented as a string) exists in the array of options (represented as the
            // generic types), we can either convert the string to the generic type and compare the values of the generic
            // type, or convert the generic types to strings and compare the strings.

            // I think it's better to compare the string representation of the generic types.

            std::stringstream ss;
            ss << options[optIdx];

            if(ss.str().compare(option) == 0) {
                // Option does exist in the options
                message = "";
                (*data) = options[optIdx];
                isValid = VALID;
                writeToGui();
                return;
            }
        }

        // Selected option does not exist in multiple choice
        std::ostringstream strs;
        strs << "Option " << option << " does not exist!";
        message = strs.str();
        isValid = INVALID;
        writeToGui();
        return;
    }

};

#endif // PARAMETERMULTIPLECHOICE_H
