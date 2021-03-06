/************************************************************************************************************************************
Copyright 2017 Autodesk, Inc. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. 
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and limitations under the License.
************************************************************************************************************************************/

#include <ai.h>
#include <algorithm>

AI_SHADER_NODE_EXPORT_METHODS(SIBScalarSmoothRangeMethods);

enum SIBScalarSmoothRangeParams
{
   p_input,
   p_min_thresh,
   p_max_thresh,
   p_min_delta,
   p_max_delta,
   p_inside_value,
   p_outside_value,
   p_invert
};

node_parameters
{
   AiParameterFlt ( "input"         , 0.5f );
   AiParameterFlt ( "min_thresh"    , 0.4f );
   AiParameterFlt ( "max_thresh"    , 0.5f );
   AiParameterFlt ( "min_delta"     , 0.05f );
   AiParameterFlt ( "max_delta"     , 0.05f );
   AiParameterFlt ( "inside_value"  , 1.0f );
   AiParameterFlt ( "outside_value" , 0.0f );
   AiParameterBool( "invert"        , false );
}

namespace {

typedef struct 
{
   bool invert;
} ShaderData;

}

node_initialize 
{
   AiNodeSetLocalData(node, AiMalloc(sizeof(ShaderData)));
}

node_update 
{
   ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
   data->invert = AiNodeGetBool(node, "invert");
}

node_finish 
{
   AiFree(AiNodeGetLocalData(node));
}

shader_evaluate
{
   ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);

   float inside_value = AiShaderEvalParamFlt(p_inside_value);
   float outside_value = AiShaderEvalParamFlt(p_outside_value);
   float input = AiShaderEvalParamFlt(p_input);
   float min_thresh = AiShaderEvalParamFlt(p_min_thresh);
   float max_thresh = AiShaderEvalParamFlt(p_max_thresh);
   float min_delta = AiShaderEvalParamFlt(p_min_delta);
   float max_delta = AiShaderEvalParamFlt(p_max_delta);

   min_delta = min_thresh - min_delta;
   max_delta = max_thresh + max_delta;

   if (min_thresh > max_thresh) 
      min_thresh = max_thresh;

   if (min_thresh < min_delta)
      std::swap(min_delta, min_thresh);
   
   if (max_thresh > max_delta)
      std::swap(max_delta, max_thresh);

   float result;
   if (input >= min_thresh && input <= max_thresh)
      result = data->invert ? outside_value : inside_value;
   else if (input<=min_delta || input>=max_delta)
      result = data->invert ? inside_value : outside_value;
   else
   {
      if (input < min_thresh)
      {
         if (input <= min_delta)
            result = 0.0f;
         else if (input >= min_thresh)
            result = 1.0f;
         else
         {
            float t = (input - min_delta) / (min_thresh - min_delta);
            float t2 = t * t;
            result = -2.0f * t2 * t + 3.0f * t2;
         }
      }
      else
      {
         if (input <= max_thresh)
            result = 0.0f;
         else if (input >= max_delta)
            result = 1.0f;
         else
         {
            float t = (input - max_thresh) / (max_delta - max_thresh);
            float t2 = t * t;
            result = -2.0f * t2 * t + 3.0f * t2;
         }

         result = 1.0f - result;
      }

      if (data->invert)
         result = inside_value * (1.0f - result) + outside_value * result;
      else
         result = outside_value * (1.0f - result) + inside_value * result;
    }

    sg->out.FLT() = result;
}
