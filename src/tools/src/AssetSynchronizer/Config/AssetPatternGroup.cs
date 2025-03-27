﻿using Utilities.Configuration;
using Newtonsoft.Json;

namespace AssetSynchronizer.Config
{
    internal sealed class AssetPatternGroup
    {
        [JsonProperty(Required = Required.Always, ItemConverterType = typeof(PathConverter))]
        public string Path { get; set; } = string.Empty;

        public List<ManifestFilter> Filters { get; set; } = new();
    }
}
