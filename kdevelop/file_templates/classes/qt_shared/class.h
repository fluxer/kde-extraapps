{% extends "cpp_header_onlyfunctions.h" %}
{% load kdev_filters %}

{% block includes %}
{{ block.super }}
#include <QSharedDataPointer>
{% endblock %}

{% block forward_declarations %}
class {{ name }}Data;
{% endblock forward_declarations %}

{% block class_body %}
{{ block.super }}

{% for member in members %}

    {{ member.type }} {{ member.name }}() const;
    void set{{ member.name|capfirst }}({{ member.type|arg_type }} {{ member.name }});

{% endfor %}

private:
    QSharedDataPointer<{{ name }}Data> d;
{% endblock class_body %}
