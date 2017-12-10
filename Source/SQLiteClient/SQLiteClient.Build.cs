using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SQLiteClient : ModuleRules
	{
		public SQLiteClient(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] 
				{ 
					"SQLiteClient/Private" 
				}
			);
            
			PublicIncludePaths.AddRange(
				new string[] 
				{ 
					"SQLiteClient/Public" 
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[] 
				{ 
					"Core",
					"Engine",
					"CoreUObject"
				}
			);
		}
	}

}