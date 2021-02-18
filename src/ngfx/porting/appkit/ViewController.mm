#include "porting/appkit/ViewController.h"
#include <MetalKit/MetalKit.h>
#include "porting/metal/MTLViewDelegate.h"

@implementation ViewController {
    MTKView *mtkView;
    MTLViewDelegate *mtkViewDelegate;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    mtkView = (MTKView *)self.view;
    mtkViewDelegate = [[MTLViewDelegate alloc] create:mtkView];
    NSAssert(mtkViewDelegate, @"Cannot create view MetalKit view delegate");
    [mtkViewDelegate mtkView:mtkView drawableSizeWillChange:mtkView.drawableSize];
    mtkView.delegate = mtkViewDelegate;
}

@end
