/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: sw=2 ts=8 et :
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Code.
 *
 * The Initial Developer of the Original Code is
 *   The Mozilla Foundation
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Chris Jones <jones.chris.g@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef mozilla_layout_RenderFrameParent_h
#define mozilla_layout_RenderFrameParent_h

#include "mozilla/layout/PRenderFrameParent.h"

#include "nsDisplayList.h"

class nsFrameLoader;

namespace mozilla {
namespace layers {
class ContainerLayer;
class Layer;
class LayerManager;
class ShadowLayersParent;
}

namespace layout {

class RenderFrameParent : public PRenderFrameParent
{
  typedef mozilla::layers::ContainerLayer ContainerLayer;
  typedef mozilla::layers::Layer Layer;
  typedef mozilla::layers::LayerManager LayerManager;
  typedef mozilla::layers::ShadowLayersParent ShadowLayersParent;

public:
  RenderFrameParent(nsFrameLoader* aFrameLoader);
  virtual ~RenderFrameParent();

  void ShadowLayersUpdated();

  already_AddRefed<Layer> BuildLayer(nsDisplayListBuilder* aBuilder,
                                     nsIFrame* aFrame,
                                     LayerManager* aManager);

protected:
  NS_OVERRIDE void ActorDestroy(ActorDestroyReason why);

  NS_OVERRIDE virtual PLayersParent* AllocPLayers();
  NS_OVERRIDE virtual bool DeallocPLayers(PLayersParent* aLayers);

private:
  LayerManager* GetLayerManager() const;
  ShadowLayersParent* GetShadowLayers() const;
  Layer* GetRootLayer() const;

  nsRefPtr<nsFrameLoader> mFrameLoader;
  nsRefPtr<ContainerLayer> mContainer;
};

} // namespace layout
} // namespace mozilla

/**
 * A DisplayRemote exists solely to graft a child process's shadow
 * layer tree (for a given RenderFrameParent) into its parent
 * process's layer tree.
 */
class nsDisplayRemote : public nsDisplayItem
{
  typedef mozilla::layout::RenderFrameParent RenderFrameParent;

public:
  nsDisplayRemote(nsIFrame* aFrame, RenderFrameParent* aRemoteFrame)
    : nsDisplayItem(aFrame)
    , mRemoteFrame(aRemoteFrame)
  {}

  NS_OVERRIDE
  virtual LayerState GetLayerState(nsDisplayListBuilder* aBuilder,
                                   LayerManager* aManager)
  { return mozilla::LAYER_ACTIVE; }  

  NS_OVERRIDE
  virtual already_AddRefed<Layer>
  BuildLayer(nsDisplayListBuilder* aBuilder, LayerManager* aManager);

  NS_DISPLAY_DECL_NAME("Remote", TYPE_REMOTE)

private:
  RenderFrameParent* mRemoteFrame;
};

#endif  // mozilla_layout_RenderFrameParent_h
