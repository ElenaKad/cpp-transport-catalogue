#include "json_builder.h"

json::Builder::Builder() {
}

json::Node json::Builder::Build() const {
    if (on_top()) {
        return root_;
    } else {
        throw std::logic_error("Returning an incomplete document");
    }
}


json::Builder::BaseContext json::Builder::Value(const json::Node & value) {
    if (in_array()) {
        nodes_stack_.back()->AsArray().push_back(value);
    } else if (in_dict()) {
        if (current_key_) {
            nodes_stack_.back()->AsDict()[current_key_.value()] = value;
            current_key_.reset();
        } else {
            throw std::logic_error("Inserting a value with no key");
        }
    } else if (on_top()) {
        root_ = value;
    } else {
        throw std::logic_error("Trying to put value while neither on top nor in array/dict");
    }
    return BaseContext(this);
}


json::Builder::DictValueContext json::Builder::Key(const std::string &key) {
    if (in_dict()) {
        current_key_ = key;
    } else {
        throw std::logic_error("Adding a key while not in dictionary");
    }
    return DictValueContext(this);
}

json::Builder::DictItemContext json::Builder::StartDict() {
    start_container(Dict());
    return DictItemContext(this);
}

json::Builder::BaseContext json::Builder::EndDict() {
    if (in_dict()) {
        nodes_stack_.pop_back();
    } else {
        throw std::logic_error("Ending a dictionary while not in dictionary");
    }
    return BaseContext(this);
}

json::Builder::ArrayItemContext json::Builder::StartArray() {
    start_container(Array());
    return ArrayItemContext(this);
}

json::Builder::BaseContext json::Builder::EndArray() {
    if (in_array()) {
        nodes_stack_.pop_back();
    } else {
        throw std::logic_error("Ending an array while not in dictionary");
    }
    return BaseContext(this);
}

void json::Builder::start_container(json::Node && container) {
    if (in_array()) {
        nodes_stack_.back()->AsArray().push_back(container);
        nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
    } else if (in_dict()) {
        if (current_key_) {
            nodes_stack_.back()->AsDict()[current_key_.value()] = container;
            nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[current_key_.value()]);
            current_key_.reset();
        } else {
            throw std::logic_error("Inserting a value with no key");
        }
    } else if (on_top()) {
        root_ = container;
        nodes_stack_.push_back(&root_);
    } else {
        throw std::logic_error("Starting an array/dict while neither on top nor in array/dict");
    }
}

bool json::Builder::on_top() const {
    return nodes_stack_.empty();
}

bool json::Builder::in_array() const {
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsArray()) {
            return true;
        }
    }
    return false;
}

bool json::Builder::in_dict() const {
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsDict()) {
            return true;
        }
    }
    return false;
}

json::Node json::Builder::BaseContext::Build() {
    return builder_->Build();
}

json::Builder::DictValueContext json::Builder::BaseContext::Key(const std::string &key)
{
    return builder_->Key(key);
}

json::Builder::DictItemContext json::Builder::BaseContext::StartDict()
{
    return builder_->StartDict();
}

json::Builder::BaseContext json::Builder::BaseContext::EndDict()
{
    return builder_->EndDict();
}

json::Builder::ArrayItemContext json::Builder::BaseContext::StartArray()
{
    return builder_->StartArray();
}

json::Builder::BaseContext json::Builder::BaseContext::EndArray()
{
    return builder_->EndArray();
}


json::Builder::BaseContext json::Builder::BaseContext::Value(const json::Node & value) {
    return builder_->Value(value);
}

json::Builder::DictItemContext json::Builder::DictValueContext::Value(const json::Node & value) {
    builder_->Value(value);
    return DictItemContext(builder_);
}

json::Builder::ArrayItemContext json::Builder::ArrayItemContext::Value(const json::Node & value) {
    builder_->Value(value);
    return ArrayItemContext(builder_);
}