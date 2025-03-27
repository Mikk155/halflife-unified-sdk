﻿using System.Xml.Serialization;

namespace MapUpgraderDocGenerator
{
    public sealed class XmlMemberDocumentation
    {
        [XmlAttribute("name")]
        public string Name { get; set; } = string.Empty;

        [XmlElement("summary")]
        public XmlSummaryElement Summary { get; set; } = new();
    }
}
