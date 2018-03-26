#include "config/parametermultiplechoice.h"

#include <QComboBox>

ParameterMultipleChoice::ParameterMultipleChoice(const string key, const string title, const std::vector<string> options, string * const data)
    : ConfigParameterBase(key, title, "", 0), data(data), options(options) {

    entryField = new QComboBox();

    // TODO: configure the combo box options

}


void ParameterMultipleChoice::parseAndValidate() {

    // TODO: read selected entry in the combo box and configure field

}

void ParameterMultipleChoice::parseAndValidate(const string option) {

    value = option;

    // Verify that the option exists in the set of options
    for(unsigned int optIdx = 0; optIdx < options.size(); optIdx++) {
        if(options[optIdx].compare(option)) {
            // Option does exist in the options
            message = "";
            (*data) = option;
            isValid = VALID;
            return;
        }
    }

    // Selected option does not exist in multiple choice
    std::ostringstream strs;
    strs << "Option " << option << " does not exist!";
    message = strs.str();
    isValid = INVALID;
    return;
}
