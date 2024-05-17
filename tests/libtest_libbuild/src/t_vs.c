#include "t_vs.h"

#include "gen/vs/vs_sln.h"

#include "common.h"
#include "test.h"

static const char *SLN_TEST = "Microsoft Visual Studio Solution File, Format Version 12.00\n"
			      "# Visual Studio Version 17\n"
			      "VisualStudioVersion = 17.4.33205.214\n"
			      "MinimumVisualStudioVersion = 10.0.40219.1\n"
			      "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"test\", \"test\\test.vcxproj\", \"{25F433E4-14EA-1031-0160-BE154E8DE8D1}\"\n"
			      "EndProject\n"
			      "Global\n"
			      "	GlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
			      "		Debug|x64 = Debug|x64\n"
			      "	EndGlobalSection\n"
			      "	GlobalSection(ProjectConfigurationPlatforms) = postSolution\n"
			      "		{25F433E4-14EA-1031-0160-BE154E8DE8D1}.Debug|x64.ActiveCfg = Debug|x64\n"
			      "		{25F433E4-14EA-1031-0160-BE154E8DE8D1}.Debug|x64.Build.0 = Debug|x64\n"
			      "	EndGlobalSection\n"
			      "	GlobalSection(SolutionProperties) = preSolution\n"
			      "		HideSolutionNode = FALSE\n"
			      "	EndGlobalSection\n"
			      "	GlobalSection(NestedProjects) = preSolution\n"
			      "	EndGlobalSection\n"
			      "	GlobalSection(ExtensibilityGlobals) = postSolution\n"
			      "		SolutionGuid = {854C0460-90C4-8BE7-BFA8-DE32843964FB}\n"
			      "	EndGlobalSection\n";

static const char *SLN_LIBTEST =
	"Microsoft Visual Studio Solution File, Format Version 12.00\n"
	"# Visual Studio Version 17\n"
	"VisualStudioVersion = 17.4.33205.214\n"
	"MinimumVisualStudioVersion = 10.0.40219.1\n"
	"Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"libtest\", \"libtest\\libtest.vcxproj\", \"{37B0CE1F-6EF7-728C-315E-DF05CB36999A}\"\n"
	"EndProject\n"
	"Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"libtest.d\", \"libtest\\libtest.d.vcxproj\", \"{B12458AA-5202-1EA6-C5FA-655A9044D549}\"\n"
	"EndProject\n"
	"Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"test\", \"test\\test.vcxproj\", \"{25F433E4-14EA-1031-0160-BE154E8DE8D1}\"\n"
	"EndProject\n"
	"Global\n"
	"	GlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
	"		Debug|x64 = Debug|x64\n"
	"	EndGlobalSection\n"
	"	GlobalSection(ProjectConfigurationPlatforms) = postSolution\n"
	"		{37B0CE1F-6EF7-728C-315E-DF05CB36999A}.Debug|x64.ActiveCfg = Debug|x64\n"
	"		{37B0CE1F-6EF7-728C-315E-DF05CB36999A}.Debug|x64.Build.0 = Debug|x64\n"
	"		{25F433E4-14EA-1031-0160-BE154E8DE8D1}.Debug|x64.ActiveCfg = Debug|x64\n"
	"		{25F433E4-14EA-1031-0160-BE154E8DE8D1}.Debug|x64.Build.0 = Debug|x64\n"
	"	EndGlobalSection\n"
	"	GlobalSection(SolutionProperties) = preSolution\n"
	"		HideSolutionNode = FALSE\n"
	"	EndGlobalSection\n"
	"	GlobalSection(NestedProjects) = preSolution\n"
	"	EndGlobalSection\n"
	"	GlobalSection(ExtensibilityGlobals) = postSolution\n"
	"		SolutionGuid = {854C0460-90C4-8BE7-BFA8-DE32843964FB}\n"
	"	EndGlobalSection\n"
	"EndGlobal\n";

static const char *PROJ_TEST_CONFIG =
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	"<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	"  <ItemGroup Label=\"ProjectConfigurations\">\n"
	"    <ProjectConfiguration Include=\"Debug|x64\">\n"
	"      <Configuration>Debug</Configuration>\n"
	"      <Platform>x64</Platform>\n"
	"    </ProjectConfiguration>\n"
	"  </ItemGroup>\n"
	"  <PropertyGroup Label=\"Globals\">\n"
	"    <VCProjectVersion>16.0</VCProjectVersion>\n"
	"    <Keyword>Win32Proj</Keyword>\n"
	"    <ProjectGuid>{25F433E4-14EA-1031-0160-BE154E8DE8D1}</ProjectGuid>\n"
	"    <RootNamespace>test</RootNamespace>\n"
	"    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>\n"
	"  </PropertyGroup>\n"
	"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n"
	"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"Configuration\">\n"
	"    <ConfigurationType>Application</ConfigurationType>\n"
	"    <UseDebugLibraries>true</UseDebugLibraries>\n"
	"    <PlatformToolset>v143</PlatformToolset>\n"
	"  </PropertyGroup>\n"
	"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
	"  <ImportGroup Label=\"ExtensionSettings\">\n"
	"  </ImportGroup>\n"
	"  <ImportGroup Label=\"Shared\">\n"
	"  </ImportGroup>\n"
	"  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n"
	"  </ImportGroup>\n"
	"  <PropertyGroup Label=\"UserMacros\" />\n"
	"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <OutDir>$(SolutionDir)bin\\$(Configuration)-$(PlatformTarget)\\test\\</OutDir>\n"
	"    <IntDir>$(SolutionDir)bin\\$(Configuration)-$(PlatformTarget)\\test\\int\\</IntDir>\n"
	"  </PropertyGroup>\n"
	"  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <ClCompile>\n"
	"      <WarningLevel>Level3</WarningLevel>\n"
	"      <SDLCheck>true</SDLCheck>\n"
	"      <ConformanceMode>true</ConformanceMode>\n";

static const char *PROJ_LIBTEST_CONFIG =
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	"<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	"  <ItemGroup Label=\"ProjectConfigurations\">\n"
	"    <ProjectConfiguration Include=\"Debug|x64\">\n"
	"      <Configuration>Debug</Configuration>\n"
	"      <Platform>x64</Platform>\n"
	"    </ProjectConfiguration>\n"
	"  </ItemGroup>\n"
	"  <PropertyGroup Label=\"Globals\">\n"
	"    <VCProjectVersion>16.0</VCProjectVersion>\n"
	"    <Keyword>Win32Proj</Keyword>\n"
	"    <ProjectGuid>{37B0CE1F-6EF7-728C-315E-DF05CB36999A}</ProjectGuid>\n"
	"    <RootNamespace>libtest</RootNamespace>\n"
	"    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>\n"
	"  </PropertyGroup>\n"
	"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n"
	"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"Configuration\">\n"
	"    <ConfigurationType>StaticLibrary</ConfigurationType>\n"
	"    <UseDebugLibraries>true</UseDebugLibraries>\n"
	"    <PlatformToolset>v143</PlatformToolset>\n"
	"  </PropertyGroup>\n"
	"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
	"  <ImportGroup Label=\"ExtensionSettings\">\n"
	"  </ImportGroup>\n"
	"  <ImportGroup Label=\"Shared\">\n"
	"  </ImportGroup>\n"
	"  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n"
	"  </ImportGroup>\n"
	"  <PropertyGroup Label=\"UserMacros\" />\n"
	"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <LinkIncremental>true</LinkIncremental>\n"
	"    <OutDir>$(SolutionDir)bin\\$(Configuration)-$(PlatformTarget)\\libtest\\</OutDir>\n"
	"    <IntDir>$(SolutionDir)bin\\$(Configuration)-$(PlatformTarget)\\libtest\\int\\</IntDir>\n"
	"  </PropertyGroup>\n"
	"  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <ClCompile>\n"
	"      <WarningLevel>Level3</WarningLevel>\n"
	"      <SDLCheck>true</SDLCheck>\n"
	"      <ConformanceMode>true</ConformanceMode>\n";

static const char *PROJ_LIBTEST_D_CONFIG =
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	"<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	"  <ItemGroup Label=\"ProjectConfigurations\">\n"
	"    <ProjectConfiguration Include=\"Debug|x64\">\n"
	"      <Configuration>Debug</Configuration>\n"
	"      <Platform>x64</Platform>\n"
	"    </ProjectConfiguration>\n"
	"  </ItemGroup>\n"
	"  <PropertyGroup Label=\"Globals\">\n"
	"    <VCProjectVersion>16.0</VCProjectVersion>\n"
	"    <Keyword>Win32Proj</Keyword>\n"
	"    <ProjectGuid>{B12458AA-5202-1EA6-C5FA-655A9044D549}</ProjectGuid>\n"
	"    <RootNamespace>libtest</RootNamespace>\n"
	"    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>\n"
	"  </PropertyGroup>\n"
	"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n"
	"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"Configuration\">\n"
	"    <ConfigurationType>DynamicLibrary</ConfigurationType>\n"
	"    <UseDebugLibraries>true</UseDebugLibraries>\n"
	"    <PlatformToolset>v143</PlatformToolset>\n"
	"  </PropertyGroup>\n"
	"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
	"  <ImportGroup Label=\"ExtensionSettings\">\n"
	"  </ImportGroup>\n"
	"  <ImportGroup Label=\"Shared\">\n"
	"  </ImportGroup>\n"
	"  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n"
	"  </ImportGroup>\n"
	"  <PropertyGroup Label=\"UserMacros\" />\n"
	"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <LinkIncremental>true</LinkIncremental>\n"
	"    <OutDir>$(SolutionDir)bin\\$(Configuration)-$(PlatformTarget)\\libtest\\</OutDir>\n"
	"    <IntDir>$(SolutionDir)bin\\$(Configuration)-$(PlatformTarget)\\libtest\\int\\</IntDir>\n"
	"  </PropertyGroup>\n"
	"  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
	"    <ClCompile>\n"
	"      <WarningLevel>Level3</WarningLevel>\n"
	"      <SDLCheck>true</SDLCheck>\n"
	"      <ConformanceMode>true</ConformanceMode>\n";

static const char *PROJ_USER = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<Project ToolsVersion=\"Current\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
			       "  <PropertyGroup>\n"
			       "    <ShowAllFiles>true</ShowAllFiles>\n"
			       "  </PropertyGroup>\n"
			       "</Project>\n";

static const char *PROJ_USER_ARGS = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
				    "<Project ToolsVersion=\"Current\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
				    "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
				    "    <LocalDebuggerCommandArguments>-D</LocalDebuggerCommandArguments>\n"
				    "  </PropertyGroup>\n"
				    "  <PropertyGroup>\n"
				    "    <ShowAllFiles>true</ShowAllFiles>\n"
				    "  </PropertyGroup>\n"
				    "</Project>\n";

TEST(c_small)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/test.sln",
			.data = SLN_TEST,
		},
		{
			.path = "tmp/test/test.vcxproj",
			.data = {
				PROJ_TEST_CONFIG,
				"      <AdditionalIncludeDirectories>$(ProjectDir)src</AdditionalIncludeDirectories>\n"
				"    </ClCompile>\n"
				"    <Link>\n"
				"      <SubSystem>Console</SubSystem>\n"
				"      <GenerateDebugInformation>true</GenerateDebugInformation>\n"
				"    </Link>\n"
				"  </ItemDefinitionGroup>\n"
				"  <ItemGroup>\n"
				"    <ClCompile Include=\"src\\main.c\" />\n"
				"  </ItemGroup>\n"
				"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
				"  <ImportGroup Label=\"ExtensionTargets\">\n"
				"  </ImportGroup>\n"
				"</Project>\n",
			},
		},
		{
			.path = "tmp/test/test.vcxproj.user",
			.data = PROJ_USER,
		}
	};

	const int ret = test_gen(vs_sln_gen, NULL, c_small_in, sizeof(c_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_args)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/test.sln",
			.data = SLN_TEST,
		},
		{
			.path = "tmp/test/test.vcxproj",
			.data = {
				PROJ_TEST_CONFIG,
				"      <AdditionalIncludeDirectories>$(ProjectDir)src</AdditionalIncludeDirectories>\n"
				"    </ClCompile>\n"
				"    <Link>\n"
				"      <SubSystem>Console</SubSystem>\n"
				"      <GenerateDebugInformation>true</GenerateDebugInformation>\n"
				"    </Link>\n"
				"  </ItemDefinitionGroup>\n"
				"  <ItemGroup>\n"
				"    <ClCompile Include=\"src\\main.c\" />\n"
				"  </ItemGroup>\n"
				"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
				"  <ImportGroup Label=\"ExtensionTargets\">\n"
				"  </ImportGroup>\n"
				"</Project>\n",
			},
		},
		{
			.path = "tmp/test/test.vcxproj.user",
			.data = PROJ_USER_ARGS,
		}
	};

	const int ret = test_gen(vs_sln_gen, NULL, c_args_in, sizeof(c_args_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_include)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/test.sln",
			.data = SLN_TEST,
		},
		{
			.path = "tmp/test/test.vcxproj",
			.data = {
				PROJ_TEST_CONFIG,
				"      <AdditionalIncludeDirectories>$(ProjectDir)include;$(ProjectDir)src</AdditionalIncludeDirectories>\n"
				"    </ClCompile>\n"
				"    <Link>\n"
				"      <SubSystem>Console</SubSystem>\n"
				"      <GenerateDebugInformation>true</GenerateDebugInformation>\n"
				"    </Link>\n"
				"  </ItemDefinitionGroup>\n"
				"  <ItemGroup>\n"
				"    <ClCompile Include=\"src\\main.c\" />\n"
				"    <ClCompile Include=\"src\\utils.c\" />\n"
				"  </ItemGroup>\n"
				"  <ItemGroup>\n"
				"    <ClInclude Include=\"include\\utils.h\" />\n"
				"  </ItemGroup>\n"
				"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
				"  <ImportGroup Label=\"ExtensionTargets\">\n"
				"  </ImportGroup>\n"
				"</Project>\n",
			},
		},
		{
			.path = "tmp/test/test.vcxproj.user",
			.data = PROJ_USER,
		}
	};

	const int ret = test_gen(vs_sln_gen, NULL, c_include_in, sizeof(c_include_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_depends)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/test.sln",
			.data = SLN_LIBTEST,
		},
		{
			.path = "tmp/libtest/libtest.vcxproj",
			.data = {
				PROJ_LIBTEST_CONFIG,
				"      <AdditionalIncludeDirectories>$(ProjectDir)src</AdditionalIncludeDirectories>\n"
				"    </ClCompile>\n"
				"    <Link>\n"
				"      <SubSystem>Console</SubSystem>\n"
				"      <GenerateDebugInformation>true</GenerateDebugInformation>\n"
				"    </Link>\n"
				"  </ItemDefinitionGroup>\n"
				"  <ItemGroup>\n"
				"    <ClCompile Include=\"src\\main.c\" />\n"
				"  </ItemGroup>\n"
				"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
				"  <ImportGroup Label=\"ExtensionTargets\">\n"
				"  </ImportGroup>\n"
				"</Project>\n",
			},
		},
		{
			.path = "tmp/libtest/libtest.d.vcxproj",
			.data = {
				PROJ_LIBTEST_D_CONFIG,
				"      <AdditionalIncludeDirectories>$(ProjectDir)src</AdditionalIncludeDirectories>\n"
				"    </ClCompile>\n"
				"    <Link>\n"
				"      <SubSystem>Console</SubSystem>\n"
				"      <GenerateDebugInformation>true</GenerateDebugInformation>\n"
				"    </Link>\n"
				"  </ItemDefinitionGroup>\n"
				"  <ItemGroup>\n"
				"    <ClCompile Include=\"src\\main.c\" />\n"
				"  </ItemGroup>\n"
				"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
				"  <ImportGroup Label=\"ExtensionTargets\">\n"
				"  </ImportGroup>\n"
				"</Project>\n",
			},
		},
		{
			.path = "tmp/libtest/libtest.vcxproj.user",
			.data = PROJ_USER,
		},
		{
			.path = "tmp/libtest/libtest.d.vcxproj.user",
			.data = PROJ_USER,
		},
		{
			.path = "tmp/test/test.vcxproj",
			.data = {
				PROJ_TEST_CONFIG,
				"      <AdditionalIncludeDirectories>$(ProjectDir)src</AdditionalIncludeDirectories>\n"
				"    </ClCompile>\n"
				"    <Link>\n"
				"      <SubSystem>Console</SubSystem>\n"
				"      <GenerateDebugInformation>true</GenerateDebugInformation>\n"
				"    </Link>\n"
				"  </ItemDefinitionGroup>\n"
				"  <ItemGroup>\n"
				"    <ClCompile Include=\"src\\main.c\" />\n"
				"  </ItemGroup>\n"
				"  <ItemGroup>\n"
				"    <ProjectReference Include=\"..\\libtest\\libtest.vcxproj\">\n"
				"      <Project>{37B0CE1F-6EF7-728C-315E-DF05CB36999A}</Project>\n"
				"    </ProjectReference>\n"
				"  </ItemGroup>\n"
				"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
				"  <ImportGroup Label=\"ExtensionTargets\">\n"
				"  </ImportGroup>\n"
				"</Project>\n",
			},
		},
		{
			.path = "tmp/test/test.vcxproj.user",
			.data = PROJ_USER,
		},
	};

	const int ret = test_gen(vs_sln_gen, NULL, c_depends_in, sizeof(c_depends_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(cpp_small)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/test.sln",
			.data = SLN_TEST,
		},
		{
			.path = "tmp/test/test.vcxproj",
			.data = {
				PROJ_TEST_CONFIG,
				"      <AdditionalIncludeDirectories>$(ProjectDir)src</AdditionalIncludeDirectories>\n"
				"    </ClCompile>\n"
				"    <Link>\n"
				"      <SubSystem>Console</SubSystem>\n"
				"      <GenerateDebugInformation>true</GenerateDebugInformation>\n"
				"    </Link>\n"
				"  </ItemDefinitionGroup>\n"
				"  <ItemGroup>\n"
				"    <ClCompile Include=\"src\\main.cpp\" />\n"
				"  </ItemGroup>\n"
				"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
				"  <ImportGroup Label=\"ExtensionTargets\">\n"
				"  </ImportGroup>\n"
				"</Project>\n",
			},
		},
		{
			.path = "tmp/test/test.vcxproj.user",
			.data = PROJ_USER,
		}
	};

	const int ret = test_gen(vs_sln_gen, NULL, cpp_small_in, sizeof(cpp_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

STEST(vs)
{
	SSTART;
	RUN(c_small);
	RUN(c_args);
	RUN(c_include);
	RUN(c_depends);
	RUN(cpp_small);
	SEND;
}
