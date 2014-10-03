using System;
using System.Reflection;
using System.Runtime.CompilerServices;

[assembly: ApplicationVersion ("2.6.2", "2.6.2")]
[assembly: ApplicationBuildInformation ("git-checkout", "linux-gnu", "i686", "2014-02-18 23:18:13 CET")]

[assembly: AssemblyVersion ("2.6.0.0")]
[assembly: AssemblyTitle ("Banshee")]
[assembly: AssemblyDescription ("Banshee Media Player")]
[assembly: AssemblyCopyright ("Copyright (C) 2005-2010 Novell Inc. and others")]
[assembly: AssemblyCompany ("Novell, Inc.")]

[AttributeUsage (AttributeTargets.Assembly, Inherited = false)]
internal sealed class ApplicationVersionAttribute : Attribute
{
    private string release_version;
    public string ReleaseVersion {
        get { return release_version; }
    }

    private string display_version;
    public string DisplayVersion {
        get { return display_version; }
    }

    public ApplicationVersionAttribute (string releaseVersion, string displayVersion)
    {
        release_version = releaseVersion;
        display_version = displayVersion;
    }
}

[AttributeUsage (AttributeTargets.Assembly, Inherited = false)]
internal sealed class ApplicationBuildInformationAttribute : Attribute
{
    private string vendor;
    public string Vendor {
        get { return vendor; }
    }

    private string host_os;
    public string HostOperatingSystem {
        get { return host_os; }
    }

    private string host_cpu;
    public string HostCpu {
        get { return host_cpu; }
    }

    private string build_time;
    public string BuildTime {
        get { return build_time; }
    }

    public ApplicationBuildInformationAttribute (string vendor, string hostOs, string hostCpu, string time)
    {
        this.vendor = vendor;
        this.host_os = hostOs;
        this.host_cpu = hostCpu;
        this.build_time = time;
    }
}

