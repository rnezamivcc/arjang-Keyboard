package android.support.v4.view;

import android.database.DataSetObserver;

//Referenced classes of package android.support.v4.view:
//         PagerAdapter

public final class VerticalViewPagerCompat
{

 private VerticalViewPagerCompat()
 {
 }

 public static void setDataSetObserver(PagerAdapter adapter, DataSetObserver observer)
 {
     adapter.registerDataSetObserver(observer);
 }
}
