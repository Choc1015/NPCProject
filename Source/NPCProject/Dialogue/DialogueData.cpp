#include "Dialogue/DialogueData.h"

const FDialogueNode* UDialogueData::FindNode(FName NodeId) const
{
	if (NodeId.IsNone())
	{
		return nullptr;
	}
	return Nodes.FindByPredicate([NodeId](const FDialogueNode& Node) { return Node.NodeId == NodeId; });
}
